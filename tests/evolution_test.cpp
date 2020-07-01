//
// Created by matteo on 30/6/20.
//
// This test checks if an entire evolutionary run works or not on a simple example
//

#include <speciation/speciation.h>
#include <speciation/Individual.h>
#include <speciation/Selection.h>
#include <speciation/Genus.h>
#include <speciation/Conf.h>
#include "catch2/catch.hpp"
#include <iostream>
#include <utility>

class Individual : public speciation::IndividualPrototype<float, Individual> {
    const int id;
    std::vector<bool> genome;
    std::optional<float> _fitness;
public:
    Individual(int id, size_t size)
        : id(id)
        , genome(size, false)
        , _fitness(std::nullopt)
    {}

    template<typename RandomGenerator>
    Individual(int id, size_t size, RandomGenerator& g)
        : id(id)
        , genome(size, false)
        , _fitness(std::nullopt)
    {
        static std::bernoulli_distribution d(0.5);
        for (size_t i=0; i<genome.size(); i++)
        {
            genome[i] = d(g);
        }
    }

    // Move constructor
    Individual(Individual &&other)
            : id(other.id)
            , genome(std::move(other.genome))
            , _fitness(other._fitness)
    {}

    float evaluate()
    {
        _fitness = 0;
        for (auto && value : genome) {
            if (value)
                (*_fitness) += 1;
        }
        return _fitness.value();
    }

    template<typename RandomGenerator>
    void mutate(RandomGenerator &g)
    {
        // flip one bit in a random position
        std::uniform_int_distribution<size_t> dis(0, genome.size() - 1);
        const size_t i = dis(g);
        genome[i] = !genome[i];
    }

    template<typename RandomGenerator>
    [[nodiscard]] Individual crossover(const Individual& other, int new_id, RandomGenerator &g) const
    {
        if (this == &other) {
            return Individual(new_id, genome); //one parent
        } else {
            std::vector<bool> mixed_genome = other.genome;
            std::uniform_int_distribution<size_t> dis(0, genome.size());
            size_t swap_point = dis(g);
            for (size_t i = swap_point; i < genome.size(); i++)
            {
                mixed_genome[i] = this->genome[i];
            }

            return Individual(new_id, std::move(mixed_genome)); //two parents
        }
    }

private:
    // Copy constructor
    Individual(const Individual &other)
        : id(other.id)
        , genome(other.genome)
        , _fitness(other._fitness)
    {}
    Individual(int id, std::vector<bool> genome)
            : id(id)
            , genome(std::move(genome))
            , _fitness(std::nullopt)
    {}

public:
    /**
     * You can implement your own fitness() function that returns a simple float.
     * The std::optional<> is just a sophistication for individuals that are not
     * evaluated yet.
     *
     * @return the fitness of the individual,
     * std::nullopt if the fitness has not been calculated yet.
     */
    [[nodiscard]] std::optional<float> fitness() const override
    {
        return _fitness;
    }

    /**
     * Tests if the this individual is compatible with another individual.
     * Compatibility means that two individuals can fit in the same species.
     * @param other
     * @return true if the two individuals are compatible.
     */
    [[nodiscard]] bool is_compatible(const Individual &other) const override
    {
        assert(genome.size() == other.genome.size());
        unsigned int distance = 0;
        for (size_t i = 0; i<genome.size(); i++) {
            if (genome[i] != other.genome[i])
                distance++;
        }

        // When more then 1/3 of elements are different, put in a different species.
        return distance > (genome.size() / 3);
    }

    /**
     * Creates a deep copy of the current individual into a new one.
     * Used in the steady state algorithm when an individual is passed as-is in the new generation.
     * It's also used when using multiple_selection_*. Because the source of multiple_selection is const.
     * @return A deep copy of `this` individual.
     */
    [[nodiscard]] virtual Individual clone() const override
    {
        return Individual(*this);
    }
};

#ifdef DEBUG
#define GENOME_SIZE 10
#define POPULATION_SIZE 10
#else
#define GENOME_SIZE 400
#define POPULATION_SIZE 100
#endif

TEST_CASE("Test evolutionary run" "[integration]")
{
    speciation::Genus<Individual,float> genus;
    std::vector<std::unique_ptr<Individual> > initial_population;
    initial_population.reserve(POPULATION_SIZE);

    std::mt19937 gen(0);

    for (size_t i=0; i<initial_population.capacity(); i++) {
        initial_population.emplace_back(std::make_unique<Individual>(i, GENOME_SIZE, gen));
    }

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

    float best_fitness = -std::numeric_limits<float>::infinity();

    srand(1);
    auto selection = [&id_counter, &gen](auto begin, auto end) {
        return speciation::tournament_selection<float>(begin, end, gen, 6);
    };
    auto parent_selection = [&id_counter](auto begin, auto end) {
        return std::make_pair(begin,begin+1);
    };
    auto crossover_1 = [&id_counter, &gen](const Individual &parent) -> std::unique_ptr<Individual> {
        return std::make_unique<Individual>(parent.crossover(parent, id_counter++, gen));
    };
    auto crossover_2 = [&id_counter, &gen](const Individual &parent_a, const Individual &parent_b) -> std::unique_ptr<Individual> {
        return std::make_unique<Individual>(parent_a.crossover(parent_b, id_counter++, gen));
    };
    auto mutate = [&gen](Individual &indiv) {
        indiv.mutate(gen);
    };
    // generational population manager
    auto population_manager = [&id_counter](std::vector<std::unique_ptr<Individual> > &&new_pop,
                                            const std::vector<const Individual*> &old_pop,
                                            unsigned int pop_amount) -> std::vector<std::unique_ptr<Individual> > {
        return std::vector<std::unique_ptr<Individual> >(std::move(new_pop));
    };
    auto evaluate = [&best_fitness](Individual *new_indiv) {
        float fitness = new_indiv->evaluate();
        if (fitness > best_fitness) {
            best_fitness = fitness;
        }
        return fitness;
    };

    unsigned int generation_n = 0;

    genus.ensure_evaluated_population(evaluate);

    try {
        while (best_fitness < GENOME_SIZE) {
            generation_n++;
            genus = genus.update(conf)
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

            if (generation_n > 1000)
            {
                FAIL("Couldn't find a solution in less than 1000 generations");
            }
        }
    } catch (const std::exception &e) {
        FAIL(e.what());
    }

    std::cout <<"Evolution took " << generation_n << " generations to complete with a fitness of " << best_fitness << std::endl;
}

#undef GENOME_SIZE
#undef POPULATION_SIZE
