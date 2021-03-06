//
// Created by matteo on 24/6/20.
//

#include <speciation/Conf.h>
#include <speciation/Selection.h>
#include <speciation/Genus.h>
#include "catch2/catch.hpp"
#include "test_individuals.h"

TEST_CASE( "Instantiate a Conf", "[conf]" )
{
    speciation::Conf conf{};
}

struct ExtendedConf : public speciation::Conf {
    std::optional<float> test_variable;
};

TEST_CASE( "Instantiate a derived conf and use it", "[conf]" )
{
    speciation::Genus<ChildIndividual,float> genus;
    std::vector<std::unique_ptr<ChildIndividual>> initial_population;// = {
    initial_population.reserve(10);
    std::mt19937 gen(0);

    initial_population.emplace_back(std::make_unique<ChildIndividual>(0));
    initial_population.emplace_back(std::make_unique<ChildIndividual>(1));
    initial_population.emplace_back(std::make_unique<ChildIndividual>(2));
    initial_population.emplace_back(std::make_unique<ChildIndividual>(3));
    initial_population.emplace_back(std::make_unique<ChildIndividual>(4));
    initial_population.emplace_back(std::make_unique<ChildIndividual>(5));
    initial_population.emplace_back(std::make_unique<ChildIndividual>(6));
    initial_population.emplace_back(std::make_unique<ChildIndividual>(7));
    initial_population.emplace_back(std::make_unique<ChildIndividual>(8));
    initial_population.emplace_back(std::make_unique<ChildIndividual>(9));

    int id_counter = static_cast<int>(initial_population.size());

    genus.speciate(initial_population.begin(), initial_population.end());
    REQUIRE(initial_population.size() == genus.count_individuals());

    const ExtendedConf conf {
        static_cast<unsigned int>(initial_population.size()),
        true,
        2,
        10,
        20,
        1.1,
        0.9,
        std::nullopt,
    };

    auto selection = [&id_counter, &gen](auto begin, auto end) {
        return speciation::tournament_selection<float>(begin, end, gen, 2);
    };
    auto parent_selection = [&id_counter](auto begin, auto end) {
        return std::make_pair(begin,begin+1);
    };
    auto crossover_1 = [&id_counter](const ChildIndividual &parent) -> std::unique_ptr<ChildIndividual> {
        return std::make_unique<ChildIndividual>(id_counter++);
    };
    auto crossover_2 = [&id_counter](const ChildIndividual &parent_a, const ChildIndividual &parent_b) -> std::unique_ptr<ChildIndividual> {
        return std::make_unique<ChildIndividual>(id_counter++);
    };
    auto mutate = [](ChildIndividual &indiv) {
        /*mutate*/
    };
    // generational population manager
    auto population_manager = [&id_counter](std::vector<std::unique_ptr<ChildIndividual> > &&new_pop,
                                            const std::vector<const ChildIndividual*> &old_pop,
                                            unsigned int pop_amount) -> std::vector<std::unique_ptr<ChildIndividual> > {
        return std::vector<std::unique_ptr<ChildIndividual> >(std::move(new_pop));
    };
    auto evaluate = [&gen](ChildIndividual *new_indiv) {
        static std::uniform_real_distribution<float> dis(0,1);
        float fit = dis(gen);
        new_indiv->set_fitness(fit);
        return fit;
    };

    genus.ensure_evaluated_population(evaluate);

    try {
        speciation::GenusSeed generated_individuals = genus.update(conf)
                .generate_new_individuals(
                        conf,
                        selection,
                        parent_selection,
                        crossover_1,
                        crossover_2,
                        mutate
                );

        generated_individuals.evaluate(evaluate);

        speciation::Genus genus1 = genus.next_generation(conf,
                                                         std::move(generated_individuals),
                                                         population_manager);
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}