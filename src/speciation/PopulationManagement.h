//
// Created by matteo on 6/24/20.
//

#ifndef SPECIATION_POPULATIONMANAGEMENT_H
#define SPECIATION_POPULATIONMANAGEMENT_H

#include <vector>
#include <functional>

namespace speciation {



template<typename I, typename F>
std::vector<I> generational(
        const std::vector<I> &new_population,
        const std::vector<I> &old_population,
        unsigned int population_size)
{
    assert(new_population.size() == population_size);
    return new_population;
}

template<typename I, typename F>
std::vector<I> steady_state(
        const std::vector<I> &new_population,
        const std::vector<I> &old_population,
        unsigned int population_size,
        std::function<I&(const std::vector<I>&)> selector)
{

}

}

#endif //SPECIATION_POPULATIONMANAGEMENT_H
