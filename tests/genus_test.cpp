//
// Created by matteo on 24/6/20.
//

#include <speciation/Selection.h>
#include "catch2/catch.hpp"
#include "speciation/Genus.h"
#include "test_individuals.h"

TEST_CASE( "Instantiate a Genus" "[genus]")
{
    speciation::Genus<Individual42,float> genus;
    REQUIRE(genus.size() == 0);
}

TEST_CASE( "Instantiate a Genus with species" "[genus]")
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

    const speciation::Conf conf {
        static_cast<unsigned int>(initial_population.size()),
        true,
        2,
        10,
        20,
        1.1,
        0.9
    };

    auto selection = [&id_counter, &gen](auto begin, auto end) {
        std::cout << "Selecting ";
        if (std::distance(begin, end) > 1) {
            auto selected = speciation::tournament_selection<float>(begin, end, gen, 2);
            std::cout << selected->individual->get_id() << std::endl;
            return selected;
        } else {
            std::cout << begin->individual->get_id() << std::endl;
            return begin;
        }
    };
    auto parent_selection = [&id_counter](auto begin, auto end) {
        std::cout << "parent_selection: " << begin->individual->get_id() << " and " << (begin+1)->individual->get_id() << std::endl;
        return std::make_pair(begin,begin+1);
    };
    auto crossover_1 = [&id_counter](const ChildIndividual &parent) -> std::unique_ptr<ChildIndividual> {
        std::cout << "crossover_1 " << parent.get_id() << "=>" << id_counter << std::endl;
        return std::make_unique<ChildIndividual>(id_counter++);
    };
    auto crossover_2 = [&id_counter](const ChildIndividual &parent_a, const ChildIndividual &parent_b) -> std::unique_ptr<ChildIndividual> {
        std::cout << "crossover_2 " << parent_a.get_id() << '+' << parent_b.get_id() << "=>" << id_counter << std::endl;
        return std::make_unique<ChildIndividual>(id_counter++);
    };
    auto mutate = [](ChildIndividual &indiv) {
        std::cout << "Mutating " << indiv.get_id() << std::endl;
        /*mutate*/
    };
    // generational population manager
    auto population_manager = [&id_counter](std::vector<std::unique_ptr<ChildIndividual> > &&new_pop,
                                            const std::vector<const ChildIndividual*> &old_pop,
                                            unsigned int pop_amount) -> std::vector<std::unique_ptr<ChildIndividual> > {
        std::cout << "population_manager " << std::endl;
        std::vector<std::unique_ptr<ChildIndividual> > result = std::vector<std::unique_ptr<ChildIndividual> >(std::move(new_pop));
        std::cout << "population_manager end" << std::endl;
        return std::move(result);
    };
    auto evaluate = [&gen](ChildIndividual *new_indiv) {
        std::cout << "Evaluating " << new_indiv->get_id() << std::endl;
        static std::uniform_real_distribution<float> dis(0,1);
        float fit = dis(gen);
        new_indiv->set_fitness(fit);
        return fit;
    };

    genus.ensure_evaluated_population(evaluate);

    try {
        std::cout << "Generation 0 updating" << std::endl;
        genus.update(conf);
        std::cout << "Generation 1 generating" << std::endl;
        speciation::Genus genus1 = genus
                .next_generation(
                        conf,
                        selection,
                        parent_selection,
                        crossover_1,
                        crossover_2,
                        mutate,
                        population_manager,
                        evaluate
                );
        std::cout << "Generation 1 done" << std::endl;
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}