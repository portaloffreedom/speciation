//
// Created by matteo on 6/22/20.
//

#ifndef SPECIATION_CONF_H
#define SPECIATION_CONF_H

namespace speciation {
struct Conf {
    /// Total population size
    unsigned int total_population_size = 100;
    /// If to enable crossover
    bool crossover = true;

    // SPECIES specific parameters

    /// when to consider a species young (inclusive)
    unsigned int young_age_threshold = 10;
    /// when to consider a species old (inclusive)
    unsigned int old_age_threshold = 40;
    /// when to consider a species stagnating (inclusive)
    unsigned int species_max_stagnation = 400;

    /// multiplier for the fitness of young species (keep > 1)
    double young_age_fitness_boost = 1.1;
    /// multiplier for the fitness of old species (keep > 0 and < 1)
    double old_age_fitness_penalty = 0.9;
};
}

#endif //SPECIATION_CONF_H
