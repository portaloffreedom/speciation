//
// Created by matteo on 6/24/20.
//

#ifndef SPECIATION_SELECTION_H
#define SPECIATION_SELECTION_H

#include <cassert>
#include <vector>
#include <functional>
#include <optional>
#include <set>
#include <sstream>
#include "Random.h"

namespace speciation {

template<typename F, typename Iter>
inline std::optional<F> standard_fitness(Iter& indiv)
{
    return (*indiv)->fitness();
}

template<typename F, typename Iter>
inline std::optional<F> indiv_fitness(Iter& indiv)
{
    return (*indiv).individual->fitness();
}

/**
 * Perform tournament selection and return best individual
 *
 * @tparam Iter points to an element that dereferences to an individual (needs ->fitness() method)
 * @param begin start of the selection pool
 * @param end end of the selection pool
 * @param k amount of individuals to participate in the tournament
 * @return tournament selected element, iterator of it
 */
template<typename F, typename Iter, typename std::optional<F> Fitness(Iter&) = indiv_fitness<F, Iter>, typename RandomGenerator >
Iter tournament_selection(const Iter begin, const Iter end, RandomGenerator &g, unsigned int k = 2)
{
    Iter best = end;
    if (begin == end) throw std::invalid_argument("Source selection cannot be empty");
    assert(k > 0);
    for (int i = 0; i < k; i++) {
        Iter candidate = select_randomly(begin, end, g);
        if (best == end || Fitness(candidate) > Fitness(best)) {
            best = candidate;
        }
    }
    assert(best != end);
    return best;
}

/**
 * Performs selection on a population of a distinct group, it can be used in the
 * form parent selection or survival selection.
 * It never selects the same individual more than once.
 *
 * @tparam IterSrc Iterator for individuals where to select from (can be const_iterator)
 * @tparam IterDest Iterator for individuals where the selection is going to be written
 * @param population_begin
 * @param population_end
 * @param destination_begin
 * @param destination_end
 * @param selection_function which function to use for selection
 */
template<typename IterSrc, typename IterDest = IterSrc, typename Selection = std::function<IterSrc(IterSrc, IterSrc)> >
void multiple_selection_no_duplicates(
        const IterSrc population_begin,
        const IterSrc population_end,
        const IterDest destination_begin,
        const IterDest destination_end,
        Selection selection_function)
{
    std::set<IterSrc> already_selected;

    auto source_size = std::distance(population_begin, population_end);
    auto dest_size = std::distance(destination_begin, destination_end);
    // If destination size is bigger than source size, this function would just
    // get stuck in an infinite loop. Better to crash here.
    // If you have this problem, you are better running the `multiple_selection_with_duplicates` function
    if (dest_size > source_size) {
        std::stringstream error_message;
        error_message << "[SOURCE (" << source_size << ") does not have enough elements to fill the DESTINATION (" << dest_size << ")]: "
                         "If destination size is bigger than source size, this function would just "
                         "get stuck in an infinite loop. Better to crash here. "
                         "If you have this problem, you are better running the `multiple_selection_with_duplicates` function";
        throw std::invalid_argument(error_message.str());
    }

    for (IterDest dest_it = destination_begin; dest_it != destination_end; ) {
        IterSrc candidate = selection_function(population_begin, population_end);

        if (already_selected.count(candidate) == 0) {
            *(*dest_it) = (*candidate)->clone();
            already_selected.insert(candidate);
            dest_it++;
        }
    }
}

/**
 * Performs selection on a population of a distinct group, it can be used in the
 * form parent selection or survival selection.
 * It can select the same individual more than once.
 *
 * @tparam IterSrc Iterator for individuals where to select from (can be const_iterator)
 * @tparam IterDest Iterator for individuals where the selection is going to be written
 * @param population_begin
 * @param population_end
 * @param destination_begin
 * @param destination_end
 * @param selection_function which function to use for selection
 */
template<typename IterSrc, typename IterDest = IterSrc, typename Selection = std::function<IterSrc(IterSrc, IterSrc)> >
void multiple_selection_with_duplicates(
        const IterSrc population_begin,
        const IterSrc population_end,
        const IterDest destination_begin,
        const IterDest destination_end,
        Selection selection_function)
{
    for (IterDest dest_it = destination_begin; dest_it != destination_end; dest_it++) {
        *(*dest_it) = (*selection_function(population_begin, population_end))->clone();
    }
}

}

#endif //SPECIATION_SELECTION_H
