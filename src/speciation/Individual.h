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
 * @tparam F fitness type, it must have a negative infinity value.
 * @tparam Individual the child individual class, for the return types of a few functions.
 */
template<typename F, typename Individual>
class IndividualPrototype {
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

    /**
     * Tests if the this individual is compatible with another individual.
     * Compatibility means that two individuals can fit in the same species.
     * @param other
     * @return true if the two individuals are compatible.
     */
    [[nodiscard]] virtual bool is_compatible(const Individual &other) const = 0;

    /**
     * Creates a deep copy of the current individual into a new one.
     * Used in the steady state algorithm when an individual is passed as-is in the new generation.
     * It's also used when using multiple_selection_*. Because the source of multiple_selection is const.
     * @return A deep copy of `this` individual.
     */
    [[nodiscard]] virtual Individual clone() const = 0;
};
}

#endif //SPECIATION_INDIVIDUAL_H
