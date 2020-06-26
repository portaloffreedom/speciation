//
// Created by matteo on 24/6/20.
//

#ifndef SPECIATION_TEST_INDIVIDUALS_H
#define SPECIATION_TEST_INDIVIDUALS_H

#include <optional>
#include <speciation/Individual.h>

struct Individual42 {
    int id;
    [[nodiscard]] float fitness() const { return 42; }
    [[nodiscard]] bool is_compatible(const Individual42 &) const
    { return rand(); }
};

struct IndividualF {
    int id;
    float _fitness;
    [[nodiscard]] float fitness() const { return _fitness; }

    bool operator==(const IndividualF &b) const {
        return id == b.id and _fitness == b._fitness;
    }
    [[nodiscard]] bool is_compatible(const Individual42 &) const
    { return rand(); }
};


struct IndividualOptionalF {
    int id;
    std::optional<float> _fitness;
    [[nodiscard]] std::optional<float> fitness() const { return _fitness; }
    [[nodiscard]] bool is_compatible(const Individual42 &) const
    { return rand(); }
};

class ChildIndividual : public speciation::Individual<float> {
protected:
    int id;
    std::optional<float> _fitness;
public:
    ChildIndividual(int id, float fitness)
            : id(id), _fitness(fitness)
    {}

    ChildIndividual(int id)
            : id(id), _fitness(std::nullopt)
    {}

    ChildIndividual() : ChildIndividual(-1)
    {}

    ChildIndividual(const ChildIndividual &other)
        : id(other.id), _fitness(other._fitness)
    {}

    ChildIndividual& operator=(const ChildIndividual &other)
    {
        id = other.id;
        _fitness = other._fitness;
        return *this;
    }

    [[nodiscard]] std::optional<float> fitness() const override
    { return _fitness; }

    void set_fitness(float fit)
    { _fitness = std::make_optional(fit); }

    [[nodiscard]] bool is_compatible(const Individual &) const override
    { return rand() % 2; }
};

class NonCopiableIndividual : public ChildIndividual {
public:
    NonCopiableIndividual(int id, float fitness)
            : ChildIndividual(id,fitness)
    {}
    NonCopiableIndividual(int id)
            : ChildIndividual(id)
    {}
    NonCopiableIndividual( const NonCopiableIndividual& ) = delete; // non construction-copyable
    NonCopiableIndividual& operator=( const NonCopiableIndividual& ) = delete; // non copyable

    bool operator==(const NonCopiableIndividual &b) const {
        return id == b.id and _fitness == b._fitness;
    }

    [[nodiscard]] std::optional<float> fitness() const override
    { return _fitness; }
};

#endif //SPECIATION_TEST_INDIVIDUALS_H
