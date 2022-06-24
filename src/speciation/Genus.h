//
// Created by matteo on 22/6/20.
//

#ifndef SPECIATION_GENUS_H
#define SPECIATION_GENUS_H

#include "SpeciesCollection.h"
#include "GenusSeed.h"
#include <forward_list>
#include <cmath>
#include <iostream>

namespace speciation {

/**
 * Collection of species
 * @tparam I individual type, it must provide a fitness through a function `std::optional<F> fitness()`
 * @tparam F fitness type, it must have a negative infinity value
 */
template <typename I, typename F>
class Genus {
    /// When creating a new species, this counter is used (and then increased).
    unsigned int next_species_id;
    /// Species Collection
    SpeciesCollection<I,F> species_collection;

public:
    /**
     * Creates a new Genus object
     */
    Genus()
            : next_species_id(1)
    {}

    /**
     * Creates a new Genus object from a pre existing SpeciesCollection
     * @param species_collection
     * @param next_species_id id of the next new species
     */
    Genus(SpeciesCollection<I, F> species_collection, unsigned int next_species_id)
            : next_species_id(next_species_id)
            , species_collection(std::move(species_collection))
    {}

    /**
     * Move constructor
     */
    Genus(Genus &&other) noexcept
            : next_species_id(other.next_species_id)
            , species_collection(std::move(other.species_collection))
    {}

    /**
     * Move assignment
     */
    Genus& operator=(Genus &&other) noexcept
    {
        if (this == &other) {
            return *this;
        }

        next_species_id = other.next_species_id;
        species_collection = std::move(other.species_collection);
        return *this;
    }

    /**
     * Creates the species. It takes a list of individuals and splits them into multiple species,
     * grouping the compatible individuals together.
     *
     * WARNING! THIS FUNCTION TAKES OWNERSHIP OF THE SOURCE ITERATOR FOR INDIVIDUALS
     *
     * @tparam Iterator non-const iterator of std::unique_ptr<I> individuals.
     * @param fist, last: the range of elements to sum
     */
    template< typename Iterator >
    void speciate(Iterator first, Iterator last) {
        // Assert a non empty iterator
        assert(first != last);

        // Clear out the species list
        species_collection.clear();

        // NOTE: we are comparing the new generation's genomes to the representative from the previous generation!
        // Any new species that is created is assigned a representative from the new generation.
        for (; first != last; first++) {
            std::unique_ptr<I> &individual = *first;
            // Iterate through each species and check if compatible. If compatible, then add the species.
            // If not compatible, create a new species.
            auto species_it = species_collection.begin();
            auto species_end = species_collection.end();
            // Find compatible species
            for (; species_it != species_end; species_it++) {
                Species<I,F> &species = *species_it;
                if (species.is_compatible(*individual)) {
                    species.insert(std::move(individual));
                    break;
                }
            }
            // No compatible species was found
            if (species_it == species_end) {
                species_collection.create_species(std::move(individual), next_species_id);
                next_species_id++;
            }
        }
    }

    void ensure_evaluated_population(const std::function<F(I*)> &evaluate_individual)
    {
        for (const Species<I, F> &species: species_collection) {
            for (const typename Species<I, F>::Indiv &i : species) {
                std::optional<F> ready_fitness = i.individual->fitness();
                if (!ready_fitness.has_value()) {
                    F fitness = evaluate_individual(i.individual.get());
                    std::optional<F> individual_fitness = i.individual->fitness();
                    assert(individual_fitness.has_value());
                    assert(fitness == individual_fitness.value());
                }
            }
        }
    }

    Genus& update(const Conf &conf)
    {
        // Update species stagnation and stuff
        species_collection.compute_update();

        // Update adjusted fitnesses
        species_collection.compute_adjust_fitness(conf);

        return *this;
    }

     /**
      * Creates the genus for the next generation.
      * The species are copied over so that `this` Genus is not invalidated.
      *
      * @param conf Species configuration object
      * @param selection function to select 1 parent (can be called even if crossover is enabled, when there is not more
      * than one parent possible)
      * @param parent_selection function to select 2 parents (only possibly called if crossover is enabled)
      * @param reproduce_individual_1 function to crossover and create new individuals from 1 parent
      * @param crossover_individual_2 function to crossover and create new individuals from 2 parents
      * @param mutate_individual function that mutates an individual
      * @param population_management function to create the new population from the old and new individual,
      * size of the new population is passed in as a parameter. The size can vary a lot from one generation to the next.
      * @param evaluate_individual function to evaluate new individuals
      * @return the genus of the next generation
      */
     GenusSeed<I,F> generate_new_individuals(
            const Conf &conf,
            const std::function<typename Species<I,F>::const_iterator (typename Species<I,F>::const_iterator, typename Species<I,F>::const_iterator)> &selection,
            const std::function<std::pair<typename Species<I,F>::const_iterator,typename Species<I,F>::const_iterator>(typename Species<I,F>::const_iterator, typename Species<I,F>::const_iterator)> &parent_selection,
            const std::function<std::unique_ptr<I>(const I&)> &reproduce_individual_1,
            const std::function<std::unique_ptr<I>(const I&, const I&)> &crossover_individual_2,
            const std::function<void(I&)> &mutate_individual
    ) const
    {

        // Calculate offspring amount
        std::vector<unsigned int> offspring_amounts = _count_offsprings(conf.total_population_size);

        // Clone Species
        SpeciesCollection<I, F> new_species_collection;
        std::vector<std::unique_ptr<I> > orphans;

        // Pointers to values in new_species_collection and orphans
        std::vector<I*> need_evaluation;

        // Pointers to current const species_collection
        std::vector<std::vector<const I*> > old_species_individuals;

        //////////////////////////////////////////////
        /// GENERATE NEW INDIVIDUALS
        int species_i = 0;
        for (const Species<I, F> &species : species_collection) {
            old_species_individuals.emplace_back(species.size());

            // Get the individuals from the individual with adjusted fitness tuple list.
            std::transform(species.cbegin(), species.cend(),
                    old_species_individuals.back().begin(),
                    [](const typename Species<I,F>::Indiv &i)
                    { return i.individual.get(); });

            std::vector<std::unique_ptr<I> > new_individuals;

            for (unsigned int n_offspring = 0; n_offspring < offspring_amounts[species_i]; n_offspring++) {
                std::unique_ptr<I> new_individual = _generate_new_individual(
                        conf,
                        species.cbegin(), species.cend(),
                        selection,
                        parent_selection,
                        reproduce_individual_1,
                        crossover_individual_2,
                        mutate_individual
                );

                // if the new individual is compatible with the species, otherwise create new.
                if (species.is_compatible(*new_individual)) {
                    new_individuals.emplace_back(std::move(new_individual));
                    need_evaluation.emplace_back(new_individuals.back().get());
                } else {
                    orphans.emplace_back(std::move(new_individual));
                    need_evaluation.emplace_back(orphans.back().get());
                }
            }

            new_species_collection.add_species(
                    species.clone_with_new_individuals(std::move(new_individuals))
                    );

            species_i++;
        }

        return GenusSeed(
                std::move(orphans),
                std::move(new_species_collection),
                std::move(need_evaluation),
                std::move(old_species_individuals));
    }

    Genus next_generation(const Conf &conf,
                          GenusSeed<I, F> &&generated_individuals,
                          const std::function<std::vector<std::unique_ptr<I> >(
                                  std::vector<std::unique_ptr<I> > &&new_individuals,
                                  const std::vector<const I *> &old_individuals,
                                  unsigned int target_population)> &population_management) const
    {
        unsigned int local_next_species_id = this->next_species_id;

        //////////////////////////////////////////////
        /// MANAGE ORPHANS, POSSIBLY CREATE NEW SPECIES
        /// recheck if other species can adopt the orphans individuals.
        std::forward_list<std::reference_wrapper<Species<I, F>>> list_of_new_species;
        for (std::unique_ptr<I> &orphan : generated_individuals.orphans) {
            bool compatible_species_found = false;
            for (Species<I, F> &species : generated_individuals.new_species_collection) {
                if (species.is_compatible(*orphan)) {
                    species.insert(std::move(orphan));
                    compatible_species_found = true;
                    break;
                }
            }
            if (!compatible_species_found) {
                Species<I, F> new_species = Species<I, F>(std::move(orphan), local_next_species_id);
                local_next_species_id++;
                generated_individuals.new_species_collection.add_species(std::move(new_species));
                // add an entry for new species which does not have a previous iteration.
                list_of_new_species.emplace_front(generated_individuals.new_species_collection.back());
            }
        }

        // Do a recount on the number of offspring per species
        unsigned int new_population_size = 0; //TODO list_of_new_species.count_individuals();
        std::vector<unsigned int> offspring_amounts = _count_offsprings(conf.total_population_size - new_population_size);
        // If this assert fails, the next population size is going to be different
        assert(std::accumulate(offspring_amounts.begin(), offspring_amounts.end(), 0u) ==
               conf.total_population_size - new_population_size);


        //////////////////////////////////////////////
        /// POPULATION MANAGEMENT
        /// update the species population, based ont he population management algorithm.
        int species_i = 0;
        for (Species<I, F> &new_species : generated_individuals.new_species_collection) {
            if (species_i > species_collection.size()) {
                // Finished. The new species keep the entire population.
                std::cout << "POPULATION MANAGEMENT Finished. The new species keep the entire population." << std::endl;
                break;
            }
            std::cout << "POPULATION MANAGEMENT " << species_i << std::endl;

            std::vector<std::unique_ptr<I> > new_species_individuals(new_species.size());
            // this empties the new_species list
            std::cout << "POPULATION MANAGEMENT " << species_i << " transform" << std::endl;
            std::transform(new_species.begin(), new_species.end(),
                           new_species_individuals.begin(),
                           [](typename Species<I, F>::Indiv &i) { return std::move(i.individual); });

            std::cout << "POPULATION MANAGEMENT " << species_i << " lambda call" << std::endl;
            // Create next population
            std::vector<std::unique_ptr<I> > new_individuals
                    = population_management(std::move(new_species_individuals),
                                            generated_individuals.old_species_individuals[species_i],
                                            offspring_amounts[species_i]);

            new_species.set_individuals(std::move(new_individuals));

            std::cout << "POPULATION MANAGEMENT " << species_i << " done" << std::endl;
            species_i++;
        }


        //////////////////////////////////////////////
        /// ASSERT SECTION
        /// check species IDs [complicated assert]
        std::set<unsigned int> species_ids;
        for (const Species<I, F> &species: generated_individuals.new_species_collection) {
            if (species_ids.count(species.id()) > 0) {
                std::stringstream error_message;
                error_message << "Species (" << species.id() << ") present twice!";
                throw std::runtime_error(error_message.str());
            }
            species_ids.insert(species.id());
        }

        generated_individuals.new_species_collection.cleanup();

        // Assert species list size and number of individuals
        size_t n_individuals = generated_individuals.new_species_collection.count_individuals();
        if (n_individuals != conf.total_population_size) {
            std::stringstream error_message;
            error_message << "count_individuals(new_species_collection) = " << n_individuals << " != "
                          << conf.total_population_size << " = population_size";
            throw std::runtime_error(error_message.str());
        }

        //////////////////////////////////////////////
        /// CREATE THE NEXT GENUS
        return Genus(std::move(generated_individuals.new_species_collection), local_next_species_id);
    }
private:
    /**
     * Generate a new individual from randomly selected parents + mutation
     *
     * @tparam Iter species population iterator (of Species::Indiv)
     * @param conf Species configuration object
     * @param population_begin start of the species population
     * @param pop_end end of the species population
     * @param selection function to select 1 parent (can be called even if crossover is enabled, when there is not more
      * than one parent possible)
     * @param parent_selection function to select 2 parents (only possibly called if crossover is enabled)
     * @param reproduce_1 function to crossover and create new individuals from 1 parent
     * @param reproduce_2 function to crossover and create new individuals from 2 parents
     * @param mutate function that mutates an individual
     * @return the genus of the next generation
     */
    template<typename Iter>
    std::unique_ptr<I> _generate_new_individual(
            const Conf &conf,
            Iter population_begin, Iter pop_end,
            const std::function<Iter(Iter, Iter)> &selection,
            const std::function<std::pair<Iter,Iter>(Iter, Iter)> &parent_selection,
            const std::function<std::unique_ptr<I>(const I&)> &reproduce_1,
            const std::function<std::unique_ptr<I>(const I&, const I&)> &reproduce_2,
            const std::function<void(I&)> &mutate) const
    {
        size_t parent_pool_size = std::distance(population_begin, pop_end);
        assert(parent_pool_size > 0);

        // Crossover
        std::unique_ptr<I> child;
        if (conf.crossover && parent_pool_size > 1) {
            std::pair<Iter,Iter> parents = parent_selection(population_begin, pop_end);
            const I &parent1 = (*parents.first->individual);
            const I &parent2 = (*parents.second->individual);
            child = reproduce_2(parent1, parent2);
        } else {
            Iter parent_iter = selection(population_begin, pop_end);
            const I &parent = (*parent_iter->individual);
            child = reproduce_1(parent);
        }

        mutate(*child);

        return std::move(child);
    }

    /**
     * Calculates the number of offsprings allocated for each individual.
     * The total of allocated individuals will be `number_of_individuals`
     *
     * @param number_of_individuals Total number of individuals to generate
     * @return a vector of integers representing the number of allocated individuals for each species.
     * The index of this list corresponds to the same index in `this->_species_list`.
     */
    std::vector<unsigned int> _count_offsprings(unsigned int number_of_individuals) const
    {
        assert(number_of_individuals > 0);

        F average_adjusted_fitness = _calculate_average_fitness();

        std::vector<unsigned int> species_offspring_amount =
                _calculate_population_size(average_adjusted_fitness);

        unsigned int offspring_amount_sum =
                std::accumulate(species_offspring_amount.begin(), species_offspring_amount.end(), 0u);
        unsigned int missing_offsprings = number_of_individuals - offspring_amount_sum;

        if (missing_offsprings != 0) {
            _correct_population_size(species_offspring_amount, missing_offsprings);
            offspring_amount_sum =
                    std::accumulate(species_offspring_amount.begin(), species_offspring_amount.end(), 0u);

            if (offspring_amount_sum != number_of_individuals) {
                std::stringstream error_message;
                error_message << "Generated species_offspring_amount (sum = " << offspring_amount_sum << ") "
                                 "does not equal number_of_individuals (" << number_of_individuals << ").\n";
                throw std::runtime_error(error_message.str());
            }
        }

        return species_offspring_amount;
    }

    /**
     * Calculates the Average fitness of the population based on the adjusted fitnesses
     * @return the average fitness
     */
    [[nodiscard]] F _calculate_average_fitness() const
    {
        // Calculate the total adjusted fitness
        F total_adjusted_fitness = 0;
        unsigned int number_of_individuals = 0;
        for (const Species<I,F> &species : species_collection) {
            for (const typename Species<I,F>::Indiv &indiv : species) {
                assert(indiv.adjusted_fitness.has_value());
                F adjusted_fitness = indiv.adjusted_fitness.value();
                total_adjusted_fitness += adjusted_fitness;
                number_of_individuals++;
            }
        }
        assert(total_adjusted_fitness > 0);

        // Calculate the average adjusted fitness
        F average_adjusted_fitness = total_adjusted_fitness / static_cast<F>(number_of_individuals);

        return average_adjusted_fitness;
    }

    /**
     * Calculates the number of offsprings allocated for each individual given the `average_adjusted_fitness`.
     * The function is rounding real numbers to integer numbers, so the returned vector quite possibly will not sum up
     * to the total population size.
     *
     * @param average_adjusted_fitness The average adjusted fitness across all the species.
     * @return a vector of integers representing the number of allocated individuals for each species.
     * The index of this list corresponds to the same index in `this->_species_list`.
     */
    [[nodiscard]] std::vector<unsigned int> _calculate_population_size(F average_adjusted_fitness) const
    {
        std::vector<unsigned int> species_offspring_amount;

        for (const Species<I,F> &species: species_collection) {
            double offspring_amount = 0.;
            for (const typename Species<I,F>::Indiv &indiv : species) {
                assert(indiv.adjusted_fitness.has_value());
                offspring_amount += indiv.adjusted_fitness.value() / average_adjusted_fitness;
            }
            species_offspring_amount.emplace_back(std::lround(offspring_amount));
        }

        return species_offspring_amount;
    }

    /**
     * `species_offspring_amount` could be incorrect because of approximation errors when we round floats to integers.
     *
     * This method modifies the `species_offspring_amount` so that the sum of the vector is equal to the total population size.
     * It adds (or removes if negative) the `missing_offspring` number of individuals in the vector.
     * When adding, it chooses the best species.
     * When removing, it chooses the worst species, multiple species if one species is not big enough.
     *
     * @param species_offspring_amount vector of offspring_amounts that needs correction
     * @param missing_offspring amount of correction to be done. Positive means we need more offsprings, negative means
     * we have to much.
     */
    void _correct_population_size(
            std::vector<unsigned int> &species_offspring_amount,
            const int missing_offspring) const
    {
        // positive means lacking individuals
        if (missing_offspring > 0)
        {
            size_t i = std::distance(species_collection.begin(), species_collection.get_best());
            species_offspring_amount[i] += missing_offspring;
        }
        // negative have excess individuals
        else if (missing_offspring < 0)
        {
            // remove missing number of individuals
            int excess_offspring = -missing_offspring;
            std::set<unsigned int> excluded_id_list;

            while (excess_offspring > 0) {
                typename std::vector<Species<I, F> >::const_iterator worst_species =
                        species_collection.get_worst(1, excluded_id_list);
                size_t worst_species_i = std::distance(species_collection.begin(), worst_species);
                int current_amount = species_offspring_amount[worst_species_i];

                if (current_amount > excess_offspring) {
                    current_amount -= excess_offspring;
                    excess_offspring = 0;
                } else {
                    excess_offspring -= current_amount;
                    current_amount = 0;
                }

                species_offspring_amount[worst_species_i] = current_amount;
                excluded_id_list.insert(worst_species->id());
            }

            assert(excess_offspring == 0);
        }
        else
        {
            std::clog << "missing_offspring == 0, why did you call _correct_population_size()?" << std::endl;
        }
    }

public:
    // Relay methods
    /**
     * Number of species
     * @return the number of species
     */
    [[nodiscard]] size_t size() const {
        return species_collection.size();
    }

    /**
     * Calculates the total number of individuals.
     * This should be equal to the population size.
     *
     * WARNING! Value is not cached and calculated every time.
     *
     * @return the total number of individuals
     */
    [[nodiscard]] size_t count_individuals() const {
        return species_collection.count_individuals();
    }

    //TODO iter_individuals

};

}

#endif //SPECIATION_GENUS_H
