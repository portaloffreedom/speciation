//
// Created by matteo on 18/6/20.
//

#include "catch2/catch.hpp"
#include "speciation/speciation.h"
#include "speciation/Species.h"
#include "speciation/exceptions.h"
#include "test_individuals.h"

using namespace speciation;

TEST_CASE("Species compile and are not broken" "[species]")
{
    std::vector<std::unique_ptr<Individual42>> initial_population;
    initial_population.emplace_back(std::make_unique<Individual42>(41));
    Species<Individual42, float> species = Species<Individual42, float>(
            initial_population.begin(), initial_population.end(),
            423
            );

    Conf conf{
            .young_age_threshold = 1,
            .old_age_threshold = 3,
            .species_max_stagnation = 100,
            .young_age_fitness_boost = 1.1,
            .old_age_fitness_penalty = 0.9,
    };

    REQUIRE(not species.empty());
    REQUIRE(species.size() == 1);
    REQUIRE(species.id() == 423);
    REQUIRE(species.representative().id == 41);
    REQUIRE(species.best_fitness() == 0);

    species.adjust_fitness(false, conf);

    REQUIRE(species.representative().id == 41);
    REQUIRE(species.best_fitness() == 42);

    species.adjust_fitness(true, conf);

    REQUIRE(species.representative().id == 41);
    REQUIRE(species.best_fitness() == 42);
}

TEST_CASE("Individuals with optional fitness" "[species]")
{
    std::unique_ptr<IndividualOptionalF> individual =
            std::make_unique<IndividualOptionalF>(41, 22);
    Species<IndividualOptionalF, float> species(
            std::move(individual),
            423);

    Conf conf{
            .young_age_threshold = 1,
            .old_age_threshold = 3,
            .species_max_stagnation = 100,
            .young_age_fitness_boost = 1.1,
            .old_age_fitness_penalty = 0.9,
    };

    REQUIRE(not species.empty());
    REQUIRE(species.size() == 1);
    REQUIRE(species.id() == 423);
    REQUIRE(species.representative().id == 41);
    REQUIRE(species.best_fitness() == 0);

    REQUIRE_NOTHROW(species.adjust_fitness(false, conf));

    REQUIRE(species.representative().id == 41);
    REQUIRE(species.best_fitness() == 22);

    REQUIRE_NOTHROW(species.adjust_fitness(true, conf));

    REQUIRE(species.representative().id == 41);
    REQUIRE(species.best_fitness() == 22);
}

TEST_CASE("Exception thrown with negative fitness" "[species]")
{
    //TODO support negative fitness in the fitness adjustment
    Species<IndividualOptionalF, float> species(
            std::make_unique<IndividualOptionalF>(451, -1),
            423);

    Conf conf{
            .young_age_threshold = 1,
            .old_age_threshold = 3,
            .species_max_stagnation = 100,
            .young_age_fitness_boost = 1.1,
            .old_age_fitness_penalty = 0.9,
    };

    REQUIRE(not species.empty());
    REQUIRE(species.size() == 1);
    REQUIRE(species.id() == 423);

    REQUIRE_THROWS_AS(species.adjust_fitness(false, conf), invalid_fitness<float>);

    REQUIRE(species.representative().id == 451);
    REQUIRE(species.best_fitness() != -1);

    REQUIRE_THROWS_AS(species.adjust_fitness(true, conf), invalid_fitness<float>);

    REQUIRE(species.representative().id == 451);
    REQUIRE(species.best_fitness() != -1);
}

TEST_CASE("Species iterator" "[species]")
{
    std::vector<std::unique_ptr<IndividualOptionalF>> individuals;
    individuals.emplace_back(std::make_unique<IndividualOptionalF>(41, 22));
    individuals.emplace_back(std::make_unique<IndividualOptionalF>(42, 21.1));
    individuals.emplace_back(std::make_unique<IndividualOptionalF>(43, 22.1));
    Species<IndividualOptionalF, float> species(
            individuals.begin(), individuals.end(),
            111);

    Conf conf{
            .young_age_threshold = 1,
            .old_age_threshold = 3,
            .species_max_stagnation = 100,
            .young_age_fitness_boost = 1.1,
            .old_age_fitness_penalty = 0.9,
    };

    REQUIRE(not species.empty());
    REQUIRE(species.size() == 3);
    REQUIRE(species.id() == 111);
    REQUIRE(species.representative().id == 41);
    REQUIRE(species.best_fitness() == 0);

    REQUIRE(species.individual(0).id == 41);
    REQUIRE(species.individual(1).id == 42);
    REQUIRE(species.individual(2).id == 43);
    REQUIRE(species.individual(0).fitness().value() == 22.f);
    REQUIRE(species.individual(1).fitness().value() == 21.1f);
    REQUIRE(species.individual(2).fitness().value() == 22.1f);
    REQUIRE_FALSE(species.adjusted_fitness(0).has_value());
    REQUIRE_FALSE(species.adjusted_fitness(1).has_value());
    REQUIRE_FALSE(species.adjusted_fitness(2).has_value());

    for (const Species<IndividualOptionalF, float>::Indiv &indiv: species) {
        REQUIRE(not indiv.adjusted_fitness.has_value());
    }

    REQUIRE_NOTHROW(species.adjust_fitness(false, conf));
    REQUIRE(species.representative().id == 41);
    REQUIRE(species.best_fitness() == 22.1f);

    REQUIRE(species.individual(0).id == 41);
    REQUIRE(species.individual(1).id == 42);
    REQUIRE(species.individual(2).id == 43);
    REQUIRE(species.individual(0).fitness().value() == 22.f);
    REQUIRE(species.individual(1).fitness().value() == 21.1f);
    REQUIRE(species.individual(2).fitness().value() == 22.1f);
    REQUIRE(species.adjusted_fitness(0).value() == Approx(8.06666f));
    REQUIRE(species.adjusted_fitness(1).value() == Approx(7.73667f));
    REQUIRE(species.adjusted_fitness(2).value() == Approx(8.10333f));

    for (Species<IndividualOptionalF, float>::Indiv &indiv: species) {
        REQUIRE(indiv.adjusted_fitness > 0.);
        (*indiv.individual->_fitness) += 100.f;
        REQUIRE(indiv.adjusted_fitness < 100.);
    }

    REQUIRE_NOTHROW(species.adjust_fitness(true, conf));
    REQUIRE(species.representative().id == 41);
    REQUIRE(species.best_fitness() == 122.1f);

    const Species<IndividualOptionalF, float> &const_species = species;
    for (const Species<IndividualOptionalF, float>::Indiv &indiv: const_species) {
        REQUIRE(indiv.adjusted_fitness.has_value());
    }

    REQUIRE(species.individual(0).id == 41);
    REQUIRE(species.individual(1).id == 42);
    REQUIRE(species.individual(2).id == 43);
    REQUIRE(species.individual(0).fitness().value() == 122.f);
    REQUIRE(species.individual(1).fitness().value() == 121.1f);
    REQUIRE(species.individual(2).fitness().value() == 122.1f);
    REQUIRE(species.adjusted_fitness(0).value() == Approx(44.73333f));
    REQUIRE(species.adjusted_fitness(1).value() == Approx(44.40333f));
    REQUIRE(species.adjusted_fitness(2).value() == Approx(44.77));
}
