//
// Created by matteo on 22/6/20.
//

#ifndef SPECIATION_GENUS_H
#define SPECIATION_GENUS_H

#include "SpeciesCollection.h"
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
    Genus(SpeciesCollection<I,F> species_collection, unsigned int next_species_id)
    : next_species_id(next_species_id)
    , species_collection(std::move(species_collection))
    {}

    /**
     * Creates the species. It takes a list of individuals and splits them into multiple species,
     * grouping the compatible individuals together.
     *
     * @tparam Iterator
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
            const I& individual = *first;
            // Iterate through each species and check if compatible. If compatible, then add the species.
            // If not compatible, create a new species.
            auto species_it = species_collection.begin();
            auto species_end = species_collection.end();
            for (; species_it != species_end; species_it++) {
                Species<I,F> &species = *species_it;
                if (species.is_compatible(individual)) {
                    species.insert(std::move(individual));
                }
            }
            // No compatible species was found
            if (species_it == species_end) {
                species_collection.add_species(Species<I,F>(individual, next_species_id));
                next_species_id++;
            }
        }
    }

    /**
     * Creates the genus for the next generation.
     * The species are copied over so that `this` Genus is not invalidated.
     *
     * @param conf
     * @param mutate_individual function that mutates an individual
     * @param crossover_individual function to crossover and create new individuals
     * @param population_management function to create the new population from the old and new individual,
     * size of the new population is passed in as a parameter. The size can vary a lot from one generation to the next.
     * @param evaluate_individual function to evaluate new individuals
     * @return the genus of the next generation
     */
    Genus next_generation(
            const Conf &conf,
            const std::function<I(typename Species<I,F>::const_iterator, typename Species<I,F>::const_iterator)> &generate_new_individual,
            const std::function<void(I&)> &mutate_individual,
            const std::function<I(const I&, const I&)> &crossover_individual,
            const std::function<std::vector<I>(const std::vector<I>&, const std::vector<I>&, unsigned int)> &population_management,
            const std::function<F(I&)> &evaluate_individual)
    {
        for (Species<I, F> &species: species_collection) {
            for (typename Species<I, F>::Indiv &i : species) {
                std::optional<F> ready_fitness = i.individual.fitness();
                if (not ready_fitness.has_value()) {
                    F fitness = evaluate_individual(i.individual);
                    assert(i.individual.fitness().has_value());
                    assert(fitness == i.individual.fitness().value());
                }
            }
        }

        // Update species stagnation and stuff
        species_collection.update();

        // Update adjusted fitnesses
        species_collection.adjust_fitness(conf);

        // Calculate offspring amount
        std::vector<unsigned int> offspring_amounts = _count_offsprings(conf.total_population_size);

        // Clone Species
        SpeciesCollection<I, F> new_species_collection;
        std::vector<std::reference_wrapper<I>> need_evaluation;
        std::vector<I> orphans;
        std::vector<std::vector<I>> old_species_individuals;

        //////////////////////////////////////////////
        /// GENERATE NEW INDIVIDUALS
        int species_i = 0;
        for (const Species<I, F> &species : species_collection) {
            old_species_individuals.emplace_back(species.size());

            // Get the individuals from the individual with adjusted fitness tuple list.
            std::transform(species.cbegin(), species.cend(),
                    old_species_individuals.back().begin(),
                    [](const typename Species<I,F>::Indiv &i)
                    { return i.individual; });

            std::vector<I> new_individuals;

            for (unsigned int n_offspring = 0; n_offspring < offspring_amounts[species_i]; n_offspring++) {
                I new_individual = generate_new_individual(species.cbegin(), species.cend());

                // if the new individual is compatible with the species, otherwise create new.
                if (species.is_compatible(new_individual)) {
                    new_individuals.emplace_back(std::move(new_individual));
                    need_evaluation.emplace_back(new_individuals.back());
                } else {
                    orphans.emplace_back(std::move(new_individual));
                    need_evaluation.emplace_back(orphans.back());
                }
            }

            new_species_collection.add_species(species.clone_with_new_individuals(std::move(new_individuals)));

            species_i++;
        }

        //////////////////////////////////////////////
        /// EVALUATE NEW INDIVIDUALS
        // TODO add here recovered individuals [partial recovery]
        for (I &new_individual : need_evaluation) {
            F fitness = evaluate_individual(new_individual);
            bool has_value = new_individual.fitness().has_value();
            assert(has_value);
            assert(fitness == new_individual.fitness().value());
        }

        //////////////////////////////////////////////
        /// MANAGE ORPHANS, POSSIBLY CREATE NEW SPECIES
        /// recheck if other species can adopt the orphans individuals.
        std::forward_list<std::reference_wrapper<Species<I, F>>> list_of_new_species;
        for (I &orphan : orphans) {
            bool compatible_species_found = false;
            for (Species<I, F> &species : new_species_collection) {
                if (species.is_compatible(orphan)) {
                    species.insert(orphan);
                    compatible_species_found = true;
                    break;
                }
            }
            if (not compatible_species_found) {
                Species<I, F> new_species = Species<I, F>({orphan}, next_species_id);
                next_species_id++;
                new_species_collection.add_species(new_species);
                // add an entry for new species which does not have a previous iteration.
                list_of_new_species.emplace_front(new_species);
            }
        }

        // Do a recount on the number of offspring per species
        size_t new_population_size = 0; //TODO list_of_new_species.count_individuals();
        offspring_amounts = _count_offsprings(conf.total_population_size - new_population_size);
        // If this assert fails, the next population size is going to be different
        assert(std::accumulate(offspring_amounts.begin(), offspring_amounts.end(), 0u) ==
               conf.total_population_size - new_population_size);


        //////////////////////////////////////////////
        /// POPULATION MANAGEMENT
        /// update the species population, based ont he population management algorithm.
        species_i = 0;
        for (Species<I, F> &new_species : new_species_collection) {
            if (species_i > species_collection.size()) {
                // Finished. The new species keep the entire population.
                break;
            }

            std::vector<I> new_species_individuals(new_species.size());
            std::transform(new_species.begin(), new_species.end(),
                           new_species_individuals.begin(),
                           [](const typename Species<I, F>::Indiv &i)
                           { return i.individual; });

            // Create next population
            std::vector<I> new_individuals
                    = population_management(new_species_individuals,
                                            old_species_individuals[species_i],
                                            offspring_amounts[species_i]);

            new_species.set_individuals(new_individuals);

            species_i++;
        }


        //////////////////////////////////////////////
        /// ASSERT SECTION
        /// check species IDs [complicated assert]
        std::set<unsigned int> species_ids;
        for (Species<I, F> &species: new_species_collection) {
            if (species_ids.count(species.id()) > 0) {
                std::stringstream error_message;
                error_message << "Species (" << species.id() << ") present twice!";
                throw std::runtime_error(error_message.str());
            }
            species_ids.insert(species.id());
        }

        new_species_collection.cleanup();

        // Assert species list size and number of individuals
        size_t n_individuals = new_species_collection.count_individuals();
        if (n_individuals != conf.total_population_size) {
            std::stringstream error_message;
            error_message << "count_individuals(new_species_collection) = " << n_individuals << " != "
                          << conf.total_population_size << " = population_size";
            throw std::runtime_error(error_message.str());
        }

        //////////////////////////////////////////////
        /// CREATE THE NEXT GENUS
        return Genus(new_species_collection, this->next_species_id);
    }
private:
    /**
     * Calculates the number of offsprings allocated for each individual.
     * The total of allocated individuals will be `number_of_individuals`
     *
     * @param number_of_individuals Total number of individuals to generate
     * @return a vector of integers representing the number of allocated individuals for each species.
     * The index of this list corresponds to the same index in `this->_species_list`.
     */
    std::vector<unsigned int> _count_offsprings(unsigned int number_of_individuals)
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
     * @return
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

        return total_adjusted_fitness;
    }

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

    void _correct_population_size(
            std::vector<unsigned int> &species_offspring_amount,
            const int missing_offspring)
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
                typename std::vector<Species<I, F>>::iterator worst_species =
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
    [[nodiscard]] size_t size() const {
        return species_collection.size();
    }

    //TODO iter_individuals

};

}

#endif //SPECIATION_GENUS_H
