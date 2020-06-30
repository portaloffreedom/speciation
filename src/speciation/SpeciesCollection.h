//
// Created by matteo on 6/22/20.
//

#ifndef SPECIATION_SPECIESCOLLECTION_H
#define SPECIATION_SPECIESCOLLECTION_H

#include <vector>
#include <set>
#include <numeric>
#include "Species.h"

namespace speciation {

/**
 * Convenient collection of all the species
 * @tparam I individual type
 * @tparam F fitness type, it must have a negative infinity value
 */
template<typename I, typename F>
class SpeciesCollection {
    std::vector<Species<I,F>> collection;
    typename std::vector<Species<I,F>>::iterator best;
    bool cache_need_updating = true;
public:
    typedef typename std::vector<Species<I, F>>::iterator iterator;
    typedef typename std::vector<Species<I, F>>::const_iterator const_iterator;

    SpeciesCollection()
        : cache_need_updating(true)
    {
        best = collection.end();
    }

    SpeciesCollection(std::vector<Species<I,F>> &&collection)
        : collection(std::move(collection))
        , cache_need_updating(true)
    {
        best = collection.end();
    }

    SpeciesCollection(SpeciesCollection &&other)
        : collection(std::move(collection))
        , best(std::move(other.best))
        , cache_need_updating(other.cache_need_updating)
    {
        assert(this != &other);
    }

    /**
     * Create a new species inline, directly inside the vector.
     * @tparam _Args Constructor parameters
     * @param __args constructor parameters
     */
    template<typename... _Args>
    void create_species(_Args&&... __args) {
        collection.emplace_back(std::forward<_Args>(__args)...);
        cache_need_updating = true;
    }

    /**
     * Moves a species inside the collection
     * @param item rvalue species to move inside the collection
     */
    void add_species(Species<I,F> &&item) {
        collection.emplace_back(std::move(item));
        cache_need_updating = true;
    }

    /**
     * Replaces the individuals of a species at index `species_index`.
     * @param species_index index of which species to operate on.
     * @param new_individuals new individuals of this species.
     */
    void set_individuals(size_t species_index, std::vector<I> new_individuals) {
        collection[species_index].set_individuals(new_individuals);
        cache_need_updating = true;
    }

    /**
     * Removes all empty species (cleanup routing for every case..)
     */
    void cleanup()
    {
        collection.erase(
                std::remove_if(collection.begin(),
                               collection.end(),
                               [](const Species<I, F> &s) {
                                   return s.empty();
                               }),
                collection.end());
    }

    /**
     * Deletes all species.
     */
    void clear()
    {
        collection.clear();
    }

    /**
     * Computes the adjusted fitness for all species
     * @param conf Species configuration object
     */
    void adjust_fitness(const Conf &conf)
    {
        for (iterator species = collection.begin();
             species != collection.end(); species++)
        {
            species->adjust_fitness(species == best, conf);
        }
    }

    /**
     * Updates the best_species, increases age for all species
     *
     * The best species gets through a rejuvenating process
     */
    void update()
    {
        // The old best species will be invalid at the first iteration
        iterator old_best = get_best();

        for (Species<I,F> &species : collection) {
            species.increase_generations();
            // This value increases continuously and it's reset every time a better fitness is found
            species.increase_no_improvements_generations();
        }

        // If the old_best is valid,
        // it's our best species and we should keep it artificially YOUNG
        if (old_best != collection.end()) {
            old_best->reset_age();
        }
    }

    /**
     * Returns an iterator pointing to the best species.
     * Be careful to not invalidate the iterator, by changing the underlying vector.
     *
     * @return the iterator pointing to the best species
     */
    iterator get_best()
    {
        assert(not collection.empty());

        if (cache_need_updating)
            _update_cache();

        return best;
    }

    /**
     * Finds the worst species (based on the best fitness of that species)
     * Crashes if there are no species with at least `minimal_size` individuals
     *
     * @param minimal_size Species with less individuals than this will not be considered
     * @param exclude_id_list Species in this list will be ignored
     * @return the iterator pointing to the worst species
     */
    iterator get_worst(size_t minimal_size, std::optional<std::set<unsigned int>> exclude_id_list = std::nullopt) {
        assert(not collection.empty());

        F worst_species_fitness = -std::numeric_limits<F>::infinity();
        typename std::vector<Species<I,F>>::iterator worst_species = collection.end();

        for (typename std::vector<Species<I,F>>::iterator species = collection.begin(); species != collection.end(); species++) {
            if (exclude_id_list.has_value() and exclude_id_list.value().count(species->id()) > 0 )
            {
                continue;
            }

            if (species->size() < minimal_size) {
                // TODO remove - this is never used since the function is only called in count_offspring.
                continue;
            }

            std::optional<F> species_fitness = species->get_best_fitness();
            if (species_fitness < worst_species_fitness) {
                assert(species_fitness.has_value());
                worst_species_fitness = species_fitness.value();
                worst_species = species;
            }
        }

        return worst_species;
    }

    /**
     * Calculates the number of individuals inside all species
     *
     * WARNING! The value is not cached and is recalculated every time.
     *
     * @return the total number of individuals across all species.
     */
    [[nodiscard]] size_t count_individuals() const
    {
        return std::accumulate(collection.begin(), collection.end(), 0,
                               [](size_t accumulator, const Species<I, F> &species) {
                                   return accumulator + species.size();
                               });
    }

private:
    /**
     * Updates the cached values
     *
     * WARNING: Cannot cache Worst value, because it's value depends on other parameters (minimal size and others)
     */
    void _update_cache() {
        assert(not collection.empty());

        // BEST
        best = std::max_element(
                collection.begin(),
                collection.end(),
                [](const Species<I,F> &a, const Species<I,F> &b)
            {
                return a.get_best_individual()->individual->fitness() < b.get_best_individual()->individual->fitness();
            }
        );

        // Cannot calculate WORST cache, because there are 2 different
        // version of the worst individual. Which one should be cached?

        cache_need_updating = false;
    }


public:
    // Relay functions
    /**
     * @return the number of species
     */
    [[nodiscard]] size_t size() const {
        return collection.size();
    }

    // iterator
    /**
     *  Returns a read/write iterator that points to the first
     *  species in the collection.
     */
    typename std::vector<Species<I,F>>::iterator begin() {
        return collection.begin();
    }

    /**
     *  Returns a read/write iterator that points one past the last
     *  species in the collection.
     */
    typename std::vector<Species<I,F>>::iterator end() {
        return collection.end();
    }

    // const iterator
    /**
     *  Returns a read-only (constant) iterator that points to the
     *  first species in the collection.
     */
    typename std::vector<Species<I,F>>::const_iterator begin() const {
        return collection.cbegin();
    }

    /**
     *  Returns a read-only (constant) iterator that points one past
     *  the last species in the collection.
     */
    typename std::vector<Species<I,F>>::const_iterator end() const {
        return collection.cend();
    }

    /**
     * The last inserted species.
     * @return reference to the last inserted species.
     */
    Species<I,F>& back() {
        return collection.back();
    }
};

}

#endif //SPECIATION_SPECIESCOLLECTION_H
