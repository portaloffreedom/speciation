//
// Created by matteo on 6/18/20.
//

#ifndef SPECIATION_INDIVIDUAL_H
#define SPECIATION_INDIVIDUAL_H

#include <optional>

namespace speciation {

// F = fitness
/**
 * Abstract class for an Individual.
 *
 * IT IS NOT MANDATORY TO USE THIS,
 * you just have to implement the same interface.
 *
 * @tparam F fitness type, it must have a negative infinity value
 */
template<typename F>
class Individual {
public:
    /**
     * You can implement your own fitness() function that returns a simple float.
     * The std::optional<> is just a sophistication for individuals that are not
     * evaluated yet.
     *
     * @return the fitness of the individual,
     * std::nullopt if the fitness has not been calculated yet.
     */
    [[nodiscard]] virtual std::optional<F> fitness() const = 0;

    [[nodiscard]] virtual bool is_compatible(const Individual &) const = 0;

//    virtual Individual clone() = 0;
};
}

#endif //SPECIATION_INDIVIDUAL_H
