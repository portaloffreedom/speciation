//
// Created by matteo on 6/24/20.
//

#ifndef SPECIATION_RANDOM_H
#define SPECIATION_RANDOM_H

#include <random>

/**
 * Select randomly from an iterator
 *
 * @tparam Iter
 * @tparam RandomGenerator
 * @param start
 * @param end
 * @param g
 * @return
 */
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

//TODO remove, use a repeatable random generator
template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}


#endif //SPECIATION_RANDOM_H
