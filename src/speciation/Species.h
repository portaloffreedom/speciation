//
// Created by matteo on 6/12/20.
//

#ifndef SPECIATION_SPECIES_H
#define SPECIATION_SPECIES_H

#include <algorithm>
#include <cassert>
#include <memory>
#include <limits>
#include <vector>
#include <iostream>
#include "Age.h"
#include "Conf.h"
#include "exceptions.h"

namespace speciation {

/**
 * Collection of individuals that are the same Species
 * I.e. they have compatible genomes and are considered similar individuals/solutions.
 * A crossover between two individuals of the same species is supposed to have a good fitness.
 *
 * @tparam I Individual type
 * @tparam F fitness type, it must have a negative infinity value
 */
template<typename I, typename F>
class Species {
public:
    struct Indiv {
        std::optional<F> adjusted_fitness;
        std::unique_ptr<I> individual;
        Indiv() = delete;
        Indiv(const Indiv&) = delete;
        Indiv& operator=(const Indiv&) = delete;

        Indiv(std::unique_ptr<I> &&indiv)
            : adjusted_fitness(std::nullopt)
            , individual(std::move(indiv))
        {}
        Indiv(Indiv&& other)
                : adjusted_fitness(other.adjusted_fitness)
                , individual(std::move(other.individual))
        {};
    };
    using iterator = typename std::vector<Indiv>::iterator;
    using const_iterator = typename std::vector<Indiv>::const_iterator;
private:

    /// List of individuals and adjusted fitness
    std::vector<Indiv> individuals;
    /// Id of the species (conserved across generations)
    unsigned int _id;
    /// `Age` of this species
    Age age;
    ///TODO documentation
    F last_best_fitness;

public:

    template <typename Iter>
    Species(Iter begin_population, Iter end_population, int species_id, Age age = Age(), F best_fitness = 0.0)
        : _id(species_id)
        , age(age)
        , last_best_fitness(best_fitness)
    {
        this->individuals.reserve(std::distance(begin_population, end_population));
        for (; begin_population != end_population; begin_population++) {
            std::unique_ptr<I> &individual = *begin_population;
            this->individuals.emplace_back(std::move(individual));
        }
    }

    Species(std::unique_ptr<I> &&individual, int species_id)
        : _id(species_id)
        , age()
        , last_best_fitness(0.0)
    {
        this->individuals.emplace_back(std::move(individual));
    }

    Species() = delete;
    Species(const Species &) = delete;
    Species(Species &&other) noexcept
            : _id(other._id)
            , age(age)
            , last_best_fitness(0.0)
            , individuals(std::move(other.individuals))
    {}

    Species& operator=(const Species &) noexcept = delete;
    Species& operator=(Species &&other) noexcept
    {
        _id = other._id;
        age = other.age;
        last_best_fitness = other.last_best_fitness;
        individuals = std::move(other.individuals);

        other._id = 0;
        other.age = Age();
        other.last_best_fitness = 0.0;
        return *this;
    }

    /**
     * Clone the current species with a new list of individuals.
     * This function is necessary to produce the new generation.
     *
     * Updating the age of the species should have already been happened before this.
     * This function will not update the age.
     *
     * @param new_individuals list of individuals that the new cloned species should have
     * @return the cloned species
     */
    Species clone_with_new_individuals(std::vector<std::unique_ptr<I> > &&new_individuals) const {
        return Species(new_individuals.begin(), new_individuals.end(), id(), age, last_best_fitness);
    }

    /**
     * Tets if the candidate individual is compatible with this Species
     * @param candidate is the individual to test against the current species
     * @return if the candidate individual is compatible or not
     */
    bool is_compatible(const I &candidate) const {
        if (this->empty())
            return false;
        return this->representative().is_compatible(candidate);
    }

    /**
     * Finds the best fitness for individuals in the species.
     * If the species is empty, it returns negative infinity.
     * @return the best fitness in the species
     */
    std::optional<F> get_best_fitness() const {
        static_assert(std::numeric_limits<F>::has_infinity, "Fitness value does not have infinity");
        if (this->empty())
            return -std::numeric_limits<F>::infinity();
        return this->get_best_individual()->individual->fitness();
    }

    /**
     * Finds the best individual in the species.
     * Crashes if the species is empty.
     * @return the best individual of the species
     */
    const_iterator get_best_individual() const {
        assert(!this->empty());

        return std::max_element(
                this->individuals.begin(),
                this->individuals.end(),
                [](const Indiv& a, const Indiv& b)
            {
                return a.individual->fitness() < b.individual->fitness();
            }
        );
    }

    /**
     * Returns a reference to the representative individual.
     * It crashes if the species is empty.
     * @return the representative individual
     */
    const I& representative() const {
        assert(!this->empty());
        return *individuals.front().individual;
    }

    /**
     * This method performs fitness sharing. It computes the adjusted fitness of the individuals.
     * It also boosts the fitness of the young and penalizes old species.
     *
     * @param is_best_species true if this is the best species
     */
    void compute_adjust_fitness(bool is_best_species, const Conf& conf) {
        assert( !this->empty() );

        // Iterates through individuals and sets the adjusted fitness (second parameter of the pair)
        for (Indiv &i: individuals) {
            std::optional<F> retrieved_fitness = i.individual->fitness();
            F fitness = retrieved_fitness.value_or(0);
            //TODO can we make this work with negative fitnesses?
            if(fitness < 0) {
                throw invalid_fitness(fitness, "Negative fitness is not supported at the moment");
            }

            fitness = this->individual_adjusted_fitness(fitness, is_best_species, conf);

            // Compute the adjusted fitness for this member
            i.adjusted_fitness = std::make_optional(fitness / individuals.size());
        }
    }

    /**
     * Inserts an individual into this species
     * @param individual
     */
    void insert(std::unique_ptr<I> &&individual) {
        this->individuals.emplace_back(std::move(individual));
    }

    /**
     * Replaces set of individuals with a new set of individuals
     */
     void set_individuals(std::vector<std::unique_ptr<I> > &&new_individuals)
     {
        std::cout << "set_individuals(this=" << id() << ')' << std::endl;
        individuals.clear();
        std::cout << "set_individuals(this=" << id() << ") reserve" << std::endl;
        individuals.reserve(new_individuals.size());
        for (std::unique_ptr<I> &individual: new_individuals) {
            std::cout << "set_individuals(individual=" << individual.get() << ") finished" << std::endl;
            individuals.emplace_back(std::move(individual));
        }
        std::cout << "set_individuals(this=" << id() << ") shrink_to_fit" << std::endl;
        individuals.shrink_to_fit();
        std::cout << "set_individuals(this=" << id() << ") finished" << std::endl;
     }

    iterator begin() {
        return this->individuals.begin();
    }

    iterator end() {
        return this->individuals.end();
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return this->individuals.cbegin();
    }

    const_iterator cend() const {
        return this->individuals.cend();
    }

private:
    /**
     * Generates the adjusted fitness (not normalized) for the individual.
     * It's based on its current fitness, the status of the species and the Configuration of the experiment.
     *
     * It also updates the this->_last_best_fitness
     *
     * @param fitness real fitness of the individual
     * @param is_best_species is this the best species in the population?
     * @return the adjusted fitness for the individual (not normalized)
     */
    F individual_adjusted_fitness(F fitness, bool is_best_species, const Conf& conf) {
        // set small fitness if it is absent
        if (fitness == 0)
            fitness = static_cast<F>(0.0001);

        // update the best fitness and stagbnation counter
        if (fitness >= this->last_best_fitness) {
            this->last_best_fitness = fitness;
            this->age.reset_no_improvements();
        }

        unsigned int number_of_generations = age.generations();
        // boost the fitness up to some young age
        if (number_of_generations < conf.young_age_threshold) {
            fitness *= static_cast<F>(conf.young_age_fitness_boost);
        }
        // penalty for old species
        if (number_of_generations > conf.old_age_threshold) {
            fitness *= static_cast<F>(conf.old_age_fitness_penalty);
        }

        // Extreme penalty if this species is stagnating for too long time
        // one exception if this is the best species found so far
        if (!is_best_species && age.no_improvements() > conf.species_max_stagnation) {
            fitness *= static_cast<F>(0.0000001);
        }

        return fitness;
    }

public:
    // Relay functions
    [[nodiscard]] bool empty() const {
        return individuals.empty();
    }

    [[nodiscard]] size_t size() const {
        return individuals.size();
    }

    void increase_generations() {
        age.increase_generations();
    }

    void increase_evaluations() {
        age.increase_evaluations();
    }

    void increase_no_improvements_generations() {
        age.increase_no_improvements();
    }

    void reset_age() {
        age.reset_generations();
        age.reset_no_improvements();
    }

    // Getters
    [[nodiscard]] unsigned int id() const
    {
        return _id;
    }
    [[nodiscard]] F best_fitness() const
    {
        return last_best_fitness;
    }
    [[nodiscard]] std::optional<F> adjusted_fitness(size_t i) const {
        return individuals.at(i).adjusted_fitness;
    }
    [[nodiscard]] I& individual(size_t i) {
        return *individuals.at(i).individual;
    }
    [[nodiscard]] const I& individual(size_t i) const {
        return *individuals.at(i).individual;
    }

    // Operators
    bool operator== (const Species<I,F> &other) const {
        if (this->id != other.id or this->age != other.age) {
            return false;
        }
        if (this->individuals.size() != other.individuals.size()) {
            return false;
        }
        for (size_t i = 0; i < individuals.size(); i++) {
            if (individuals[i].adjusted_fitness != other.individuals[i].adjusted_fitness)
                return false;
            if (individuals[i].individual != other.individuals[i].individual)
                return false;
        }
        return true;
    }

    Indiv &operator[](size_t i) {
        return individuals[i];
    }
};

}


#endif //SPECIATION_SPECIES_H
