//
// Created by matteo on 24/6/20.
//

#include <iostream>
#include <forward_list>
#include "catch2/catch.hpp"
#include "speciation/Selection.h"
#include "test_individuals.h"

using namespace speciation;

TEST_CASE("Tournament selection" "[selection]")
{
    std::vector<IndividualF> population = {{1, 1},
                                           {2, 2},
                                           {3, 3}};

    std::vector<IndividualF>::iterator candidate
            = speciation::tournament_selection<float>(population.begin(), population.end());
    REQUIRE(std::find(population.begin(), population.end(), *candidate) != population.end());

    std::vector<IndividualF>::const_iterator const_candidate
            = speciation::tournament_selection<float>(population.begin(), population.end());
    REQUIRE(std::find(population.begin(), population.end(), *const_candidate) != population.end());

    IndividualF candidate_copy
            = *speciation::tournament_selection<float>(population.begin(), population.end());
}

std::optional<float> fitness_ref_wrapper(std::vector<std::reference_wrapper<NonCopiableIndividual>>::iterator &it)
{
    return it->get().fitness();
}

TEST_CASE("Tournament selection with a non copiable individual" "[selection]")
{
    //std::vector requires its inside elements to be copiable
    std::forward_list<NonCopiableIndividual> population;
    population.emplace_front(1, 1.);
    population.emplace_front(2, 2.);
    population.emplace_front(3, 3.);

    std::forward_list<NonCopiableIndividual>::iterator candidate
            = speciation::tournament_selection<float>(population.begin(), population.end());
    REQUIRE(std::find(population.begin(), population.end(), *candidate) != population.end());

    std::forward_list<NonCopiableIndividual>::const_iterator const_candidate
            = speciation::tournament_selection<float>(population.begin(), population.end());
    REQUIRE(std::find(population.begin(), population.end(), *const_candidate) != population.end());

    std::vector<std::reference_wrapper<NonCopiableIndividual>> population_vec;
    for (NonCopiableIndividual &indiv : population) {
        population_vec.emplace_back(indiv);
    }

    std::vector<std::reference_wrapper<NonCopiableIndividual>>::iterator candidate_vec
            = speciation::tournament_selection<float, std::vector<std::reference_wrapper<NonCopiableIndividual>>::iterator, fitness_ref_wrapper>
                    (population_vec.begin(), population_vec.end());
    REQUIRE(std::find(population.begin(), population.end(), *candidate_vec) != population.end());
}

TEST_CASE("Tournament selection exception on empty set" "[selection]")
{
    std::vector<IndividualF> population = {};
    REQUIRE_THROWS_AS(
            speciation::tournament_selection<float>(population.begin(), population.end()),
            std::invalid_argument
    );
}

TEST_CASE("Tournament selection should be able to all elements" "[selection]")
{
    std::vector<IndividualF> population = {{1, 1},
                                           {2, 2},
                                           {3, 3}};
    std::set<unsigned int> found;
    int i;
    for (i = 0; i < 100; i++) {
        IndividualF candidate
                = *speciation::tournament_selection<float>(population.begin(), population.end());
        found.insert(candidate.id);
        if (found.size() == population.size())
            break;
    }
    // All elements where found
    REQUIRE(found.size() == population.size());
    if (i > 50) {
        std::clog << "WARNING, it took more than 50 iterations to find all 3 elements" << std::endl;
    }
}

TEST_CASE("Multiple selection with duplicates" "[selection]")
{
    std::vector<IndividualF> source = {{1, 1},
                                       {2, 2},
                                       {3, 3}};
    std::vector<IndividualF> destination = {{},{},{},{}};

    // This is mostly a compilation test on how templates are working

    multiple_selection_with_duplicates<typename std::vector<IndividualF>::const_iterator, typename std::vector<IndividualF>::iterator>(
            source.cbegin(), source.cend(),
            destination.begin(), destination.end(),
            [](auto begin,
               auto end) {
                return speciation::tournament_selection<float>(begin, end, 3);
            }
    );

    multiple_selection_with_duplicates<typename std::vector<IndividualF>::iterator>(
            source.begin(), source.end(),
            destination.begin(), destination.end(),
            [](auto begin,
               auto end) {
                return speciation::tournament_selection<float>(begin, end, 3);
            }
    );

    multiple_selection_with_duplicates(
            source.begin(), source.end(),
            destination.begin(), destination.end(),
            [](std::vector<IndividualF>::iterator begin,
               std::vector<IndividualF>::iterator end) {
                return speciation::tournament_selection<float>(begin, end, 3);
            }
    );

    multiple_selection_with_duplicates(
            source.begin(), source.end(),
            destination.begin(), destination.end(),
            [](auto begin, auto end) {
                return speciation::tournament_selection<float>(begin, end, 3);
            }
    );
}

TEST_CASE("Multiple selection with no duplicates" "[selection]")
{
    std::vector<IndividualF> source = {{1, 1},
                                       {2, 2},
                                       {3, 3}};
    std::vector<IndividualF> destination = {{},{},{}};

    // This is mostly a compilation test on how templates are working

    REQUIRE_NOTHROW(
            multiple_selection_no_duplicates<typename std::vector<IndividualF>::const_iterator, typename std::vector<IndividualF>::iterator>(
                    source.cbegin(), source.cend(),
                    destination.begin(), destination.end(),
                    [](auto begin,
                       auto end) {
                        return speciation::tournament_selection<float>(begin, end, 3);
                    }
            ));

    REQUIRE_NOTHROW(
            multiple_selection_no_duplicates<typename std::vector<IndividualF>::iterator>(
                    source.begin(), source.end(),
                    destination.begin(), destination.end(),
                    [](auto begin,
                       auto end) {
                        return speciation::tournament_selection<float>(begin, end, 3);
                    }
            ));

    REQUIRE_NOTHROW(
            multiple_selection_no_duplicates(
                    source.begin(), source.end(),
                    destination.begin(), destination.end(),
                    [](std::vector<IndividualF>::iterator begin,
                       std::vector<IndividualF>::iterator end) {
                        return speciation::tournament_selection<float>(begin, end, 3);
                    }
            ));

    REQUIRE_NOTHROW(
            multiple_selection_no_duplicates(
                    source.begin(), source.end(),
                    destination.begin(), destination.end(),
                    [](auto begin, auto end) {
                        return speciation::tournament_selection<float>(begin, end, 3);
                    }
            ));

}

TEST_CASE("Multiple selection with no duplicates should throw an exception when destination is too big" "[selection]")
{
    std::vector<IndividualF> source = {{1, 1},
                                       {2, 2},
                                       {3, 3}};
    std::vector<IndividualF> destination(4);
    REQUIRE_THROWS_AS(
            multiple_selection_no_duplicates(
                    source.begin(), source.end(),
                    destination.begin(), destination.end(),
                    [](auto begin, auto end) {
                        return speciation::tournament_selection<float>(begin, end, 3);
                    }
            ), std::invalid_argument);
}
