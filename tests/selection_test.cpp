//
// Created by matteo on 24/6/20.
//

#include <iostream>
#include <forward_list>
#include "catch2/catch.hpp"
#include "speciation/Selection.h"
#include "test_individuals.h"

TEST_CASE("Tournament selection" "[selection]")
{
    typedef std::vector<std::unique_ptr<IndividualF> >::iterator Iter;
    typedef std::vector<std::unique_ptr<IndividualF> >::const_iterator CIter;
    std::mt19937 gen(0);

    std::vector<std::unique_ptr<IndividualF> > population;
    population.emplace_back(std::make_unique<IndividualF>(1, 1));
    population.emplace_back(std::make_unique<IndividualF>(2, 2));
    population.emplace_back(std::make_unique<IndividualF>(3, 3));


    Iter candidate
            = speciation::tournament_selection<float, Iter, speciation::standard_fitness>(population.begin(), population.end(), gen);
    REQUIRE(std::find(population.begin(), population.end(), *candidate) != population.end());

    CIter const_candidate
            = speciation::tournament_selection<float, CIter, speciation::standard_fitness>(population.begin(), population.end(), gen);
    REQUIRE(std::find(population.begin(), population.end(), *const_candidate) != population.end());

//    std::unique_ptr<IndividualF> candidate_copy
//            = speciation::tournament_selection<float>(population.begin(), population.end());
}

TEST_CASE("Tournament selection points to the original source" "[selection]")
{
    typedef std::vector<std::unique_ptr<IndividualF> >::iterator Iter;
    typedef std::vector<std::unique_ptr<IndividualF> >::const_iterator CIter;
    std::mt19937 gen(0);

    std::vector<std::unique_ptr<IndividualF> > population;
    population.emplace_back(std::make_unique<IndividualF>(1, 1));
    population.emplace_back(std::make_unique<IndividualF>(2, 2));
    population.emplace_back(std::make_unique<IndividualF>(3, 3));

    Iter candidate
            = speciation::tournament_selection<float, Iter, speciation::standard_fitness>(population.begin(), population.end(), gen);
    REQUIRE(std::find(population.begin(), population.end(), *candidate) != population.end());

    std::unique_ptr<IndividualF> moved_individual = std::move(*candidate);
    REQUIRE(moved_individual != nullptr);
    REQUIRE(std::count(population.begin(), population.end(), nullptr) == 1);
}

std::optional<float>
fitness_ref_wrapper(std::vector<std::reference_wrapper<std::unique_ptr<NonCopiableIndividual>>>::iterator &it)
{
    return it->get()->fitness();
}

TEST_CASE("Tournament selection with a non copiable individual" "[selection]")
{
    typedef std::forward_list<std::unique_ptr<NonCopiableIndividual> >::iterator Iter;
    typedef std::forward_list<std::unique_ptr<NonCopiableIndividual> >::const_iterator CIter;
    std::mt19937 gen(0);

    //std::vector requires its inside elements to be copiable
    std::forward_list<std::unique_ptr<NonCopiableIndividual> > population;
    population.emplace_front(std::make_unique<NonCopiableIndividual>(1, 1.));
    population.emplace_front(std::make_unique<NonCopiableIndividual>(2, 2.));
    population.emplace_front(std::make_unique<NonCopiableIndividual>(3, 3.));

    Iter candidate
            = speciation::tournament_selection<float, Iter, speciation::standard_fitness>(population.begin(), population.end(), gen);
    REQUIRE(std::find(population.begin(), population.end(), *candidate) != population.end());

    CIter const_candidate
            = speciation::tournament_selection<float, CIter, speciation::standard_fitness>(population.begin(), population.end(), gen);
    REQUIRE(std::find(population.begin(), population.end(), *const_candidate) != population.end());

    std::vector<std::reference_wrapper<std::unique_ptr<NonCopiableIndividual>>> population_vec;
    for (std::unique_ptr<NonCopiableIndividual> &indiv : population) {
        population_vec.emplace_back(indiv);
    }

    std::vector<std::reference_wrapper<std::unique_ptr<NonCopiableIndividual>>>::iterator candidate_vec
            = speciation::tournament_selection
                    <
                            float,
                            typename std::vector<std::reference_wrapper<std::unique_ptr<NonCopiableIndividual>>>::iterator,
                            fitness_ref_wrapper
                    >
                    (population_vec.begin(), population_vec.end(), gen);
    REQUIRE(std::find(population.begin(), population.end(), (*candidate_vec).get()) != population.end());
}

TEST_CASE("Tournament selection exception on empty set" "[selection]")
{
    typedef std::vector<std::unique_ptr<IndividualF> >::iterator Iter;
    typedef std::vector<std::unique_ptr<IndividualF> >::const_iterator CIter;
    std::mt19937 gen(0);

    constexpr auto tournament = &speciation::tournament_selection<float, Iter, speciation::standard_fitness, std::mt19937>;

    std::vector<std::unique_ptr<IndividualF> > population = {};
    REQUIRE_THROWS_AS(
            tournament(population.begin(), population.end(), gen, 2),
            std::invalid_argument
    );
}

TEST_CASE("Tournament selection should be able to all elements" "[selection]")
{
    typedef std::vector<std::unique_ptr<IndividualF> >::iterator Iter;
    typedef std::vector<std::unique_ptr<IndividualF> >::const_iterator CIter;
    std::mt19937 gen(0);

    std::vector<std::unique_ptr<IndividualF> > population;
    population.emplace_back(std::make_unique<IndividualF>(1, 1));
    population.emplace_back(std::make_unique<IndividualF>(2, 2));
    population.emplace_back(std::make_unique<IndividualF>(3, 3));

    std::set<unsigned int> found;
    int i;
    for (i = 0; i < 100; i++) {
        std::unique_ptr<IndividualF> &candidate
                = *speciation::tournament_selection<float, Iter, speciation::standard_fitness>(population.begin(), population.end(), gen);
        found.insert(candidate->id);
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
    typedef std::vector<std::unique_ptr<IndividualF> >::iterator Iter;
    typedef std::vector<std::unique_ptr<IndividualF> >::const_iterator CIter;
    std::mt19937 gen(0);

    std::vector<std::unique_ptr<IndividualF> > source;
    source.emplace_back(std::make_unique<IndividualF>(1, 1));
    source.emplace_back(std::make_unique<IndividualF>(2, 2));
    source.emplace_back(std::make_unique<IndividualF>(3, 3));

    std::vector<std::unique_ptr<IndividualF> > destination(4);

    // This is mostly a compilation test on how templates are working

    speciation::multiple_selection_with_duplicates<CIter, Iter>(
            source.cbegin(), source.cend(),
            destination.begin(), destination.end(),
            [&gen](CIter begin,
               CIter end) {
                return speciation::tournament_selection<float, CIter, speciation::standard_fitness>(begin, end, gen, 3);
            }
    );

    speciation::multiple_selection_with_duplicates<Iter>(
            source.begin(), source.end(),
            destination.begin(), destination.end(),
            [&gen](Iter begin,
               Iter end) {
                return speciation::tournament_selection<float, Iter, speciation::standard_fitness>(begin, end, gen, 3);
            }
    );

    speciation::multiple_selection_with_duplicates(
            source.begin(), source.end(),
            destination.begin(), destination.end(),
            [&gen](std::vector<std::unique_ptr<IndividualF> >::iterator begin,
               std::vector<std::unique_ptr<IndividualF> >::iterator end) {
                return speciation::tournament_selection<float, CIter, speciation::standard_fitness>(begin, end, gen, 3);
            }
    );

    speciation::multiple_selection_with_duplicates(
            source.begin(), source.end(),
            destination.begin(), destination.end(),
            [&gen](auto begin, auto end) {
                return speciation::tournament_selection<float, CIter, speciation::standard_fitness>(begin, end, gen, 3);
            }
    );
}

TEST_CASE("Multiple selection with no duplicates" "[selection]")
{
    typedef std::vector<std::unique_ptr<IndividualF> >::iterator Iter;
    typedef std::vector<std::unique_ptr<IndividualF> >::const_iterator CIter;
    std::mt19937 gen(0);

    std::vector<std::unique_ptr<IndividualF> > source;
    source.emplace_back(std::make_unique<IndividualF>(1, 1));
    source.emplace_back(std::make_unique<IndividualF>(2, 2));
    source.emplace_back(std::make_unique<IndividualF>(3, 3));

    std::vector<std::unique_ptr<IndividualF> > destination(3);

    // This is mostly a compilation test on how templates are working

    REQUIRE_NOTHROW(
            speciation::multiple_selection_no_duplicates<CIter, Iter>(
                    source.cbegin(), source.cend(),
                    destination.begin(), destination.end(),
                    [&gen](auto begin,
                       auto end) {
                        return speciation::tournament_selection<float, CIter, speciation::standard_fitness>(begin, end, gen, 3);
                    }
            ));

    REQUIRE_NOTHROW(
            speciation::multiple_selection_no_duplicates<Iter>(
                    source.begin(), source.end(),
                    destination.begin(), destination.end(),
                    [&gen](auto begin,
                       auto end) {
                        return speciation::tournament_selection<float, Iter, speciation::standard_fitness>(begin, end, gen, 3);
                    }
            ));

    REQUIRE_NOTHROW(
            speciation::multiple_selection_no_duplicates(
                    source.begin(), source.end(),
                    destination.begin(), destination.end(),
                    [&gen](Iter begin,
                       Iter end) {
                        return speciation::tournament_selection<float, Iter, speciation::standard_fitness>(begin, end, gen, 3);
                    }
            ));

    REQUIRE_NOTHROW(
            speciation::multiple_selection_no_duplicates(
                    source.begin(), source.end(),
                    destination.begin(), destination.end(),
                    [&gen](auto begin, auto end) {
                        return speciation::tournament_selection<float, Iter, speciation::standard_fitness>(begin, end, gen, 3);
                    }
            ));

}

TEST_CASE("Multiple selection with no duplicates should throw an exception when destination is too big" "[selection]")
{
    typedef std::vector<std::unique_ptr<IndividualF> >::iterator Iter;
    typedef std::vector<std::unique_ptr<IndividualF> >::const_iterator CIter;
    std::mt19937 gen(0);

    std::vector<std::unique_ptr<IndividualF> > source;
    source.emplace_back(std::make_unique<IndividualF>(1, 1));
    source.emplace_back(std::make_unique<IndividualF>(2, 2));
    source.emplace_back(std::make_unique<IndividualF>(3, 3));

    std::vector<std::unique_ptr<IndividualF> > destination(4);
    REQUIRE_THROWS_AS(
            speciation::multiple_selection_no_duplicates(
                    source.begin(), source.end(),
                    destination.begin(), destination.end(),
                    [&gen](auto begin, auto end) {
                        return speciation::tournament_selection<float, Iter, speciation::standard_fitness>(begin, end, gen, 3);
                    }
            ), std::invalid_argument);
}
