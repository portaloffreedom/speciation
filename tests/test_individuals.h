//
// Created by matteo on 24/6/20.
//

#ifndef SPECIATION_TEST_INDIVIDUALS_H
#define SPECIATION_TEST_INDIVIDUALS_H

#include <optional>
#include <speciation/Individual.h>

struct Individual42 {
    int id;
    Individual42(int id) : id(id) {}
    Individual42 clone() const { return Individual42(id); }
    [[nodiscard]] float fitness() const { return 42; }
    [[nodiscard]] bool is_compatible(const Individual42 &) const
    { return rand(); }
};

struct IndividualF {
    int id;
    float _fitness;
    IndividualF(int id, float f) : id(id), _fitness(f) {}
    [[nodiscard]] IndividualF clone() const {
        return IndividualF(id,_fitness);
    }
    bool operator==(const IndividualF &b) const {
        return id == b.id && _fitness == b._fitness;
    }
    IndividualF& operator=(const IndividualF &other) {
        return *this;
    }

    [[nodiscard]] float fitness() const { return _fitness; }
    [[nodiscard]] bool is_compatible(const Individual42 &) const
    { return rand(); }
};


struct IndividualOptionalF {
    int id;
    std::optional<float> _fitness;
    IndividualOptionalF(int id) : id(id), _fitness(std::nullopt) {}
    IndividualOptionalF(int id, float f) : id(id), _fitness(f) {}
    [[nodiscard]] IndividualOptionalF clone() const {
        IndividualOptionalF i = IndividualOptionalF(id);
        i._fitness = _fitness;
        return i;
    }
    [[nodiscard]] std::optional<float> fitness() const { return _fitness; }
    [[nodiscard]] bool is_compatible(const Individual42 &) const
    { return rand(); }
};

class ChildIndividual : public speciation::IndividualPrototype<float,ChildIndividual> {
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

    bool operator==(const ChildIndividual &b) const {
        return id == b.id && _fitness == b._fitness;
    }

    [[nodiscard]] std::optional<float> fitness() const override
    { return _fitness; }

    [[nodiscard]] int get_id() const
    { return this->id; }

    void set_fitness(float fit)
    { _fitness = fit; }

    void set_id(int id)
    { this->id = id; }

    [[nodiscard]] bool is_compatible(const ChildIndividual &) const override
    { return rand() % 2; }

    [[nodiscard]] ChildIndividual clone() const
    { return ChildIndividual(*this); }
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

};

#endif //SPECIATION_TEST_INDIVIDUALS_H
