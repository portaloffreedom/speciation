//
// Created by matteo on 24/6/20.
//

#include "catch2/catch.hpp"
#include "speciation/Genus.h"
#include "test_individuals.h"

using namespace speciation;

TEST_CASE( "Instantiate a Genus" "[genus]")
{
    Genus<Individual42,float> genus;
    REQUIRE(genus.size() == 0);
}

TEST_CASE( "Instantiate a Genus with species" "[genus]")
{
    Genus<ChildIndividual,float> genus;
    std::vector<ChildIndividual> initial_population = {
            {0},
            {1},
            {2},
            {3},
            {4},
            {5},
            {6},
            {7},
            {8},
            {9},
    };

    int id_counter = 10;

    genus.speciate(initial_population.cbegin(), initial_population.cend());
    const Conf conf {
        10,
        2,
        10,
        20,
        1.1,
        0.9
    };

    srand(1);

    auto generate_new_individual = [&id_counter](auto begin, auto end) {
        return ChildIndividual(id_counter++);
    };
    auto mutate = [](ChildIndividual &indiv) {
        /*mutate*/
    };
    auto crossover = [&id_counter](const ChildIndividual &a, const ChildIndividual &b) {
        return ChildIndividual(id_counter++);
    };
    auto population_manager = [&id_counter](const std::vector<ChildIndividual> &new_pop,
                                            const std::vector<ChildIndividual> &old_pop,
                                            unsigned int pop_amount) {
        return std::vector<ChildIndividual>(pop_amount);
    };
    auto evaluate = [](ChildIndividual &new_indiv) {
        float fit = float(rand()) / float(RAND_MAX);
        new_indiv.set_fitness(fit);
        return fit;
    };

    try {
        Genus genus1 = genus.next_generation(
                conf,
                generate_new_individual,
                mutate,
                crossover,
                population_manager,
                evaluate
        );
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}