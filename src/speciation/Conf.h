//
// Created by matteo on 6/22/20.
//

#ifndef SPECIATION_CONF_H
#define SPECIATION_CONF_H

namespace speciation {
struct Conf {
    unsigned int total_population_size;

    unsigned int young_age_threshold;
    unsigned int old_age_threshold;
    unsigned int species_max_stagnation;

    double young_age_fitness_boost;
    double old_age_fitness_penalty;
};
}

#endif //SPECIATION_CONF_H
