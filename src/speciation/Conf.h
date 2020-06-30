//
// Created by matteo on 6/22/20.
//

#ifndef SPECIATION_CONF_H
#define SPECIATION_CONF_H

namespace speciation {
struct Conf {
    /// Total population size
    unsigned int total_population_size;
    /// If to enable crossover
    bool crossover;

    // SPECIES specific parameters

    /// when to consider a species young (inclusive)
    unsigned int young_age_threshold;
    /// when to consider a species old (inclusive)
    unsigned int old_age_threshold;
    /// when to consider a species stagnating (inclusive)
    unsigned int species_max_stagnation;

    /// multiplier for the fitness of young species (keep > 1)
    double young_age_fitness_boost;
    /// multiplier for the fitness of old species (keep > 0 and < 1)
    double old_age_fitness_penalty;
};
}

#endif //SPECIATION_CONF_H
