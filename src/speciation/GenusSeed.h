//
// Created by matteo on 7/7/20.
//

#ifndef SPECIATION_GENUSSEED_H
#define SPECIATION_GENUSSEED_H

#include "SpeciesCollection.h"

#include <vector>
#include <memory>

namespace speciation {

template<typename I, typename F>
class Genus;

template<typename I, typename F>
class GenusSeed {
    friend class Genus<I,F>;
private:
    std::vector<std::unique_ptr<I> > orphans;
    SpeciesCollection<I, F> new_species_collection;
    std::vector<I*> need_evaluation;
    const std::vector<std::vector<const I*> > old_species_individuals;
public:

    typename std::vector<I*>::iterator begin()
    {
        return need_evaluation.begin();
    }

    typename std::vector<I*>::iterator end()
    {
        return need_evaluation.end();
    }

    //////////////////////////////////////////////
    /// EVALUATE NEW INDIVIDUALS
    void evaluate(const std::function<F(I*)> &evaluate_individual)
    {
        for (I *new_individual : *this) {
            F fitness = evaluate_individual(new_individual);
            std::optional<F> individual_fitness = new_individual->fitness();
            assert(individual_fitness.has_value());
            assert(fitness == individual_fitness.value());
        }
    }

private:
    GenusSeed(std::vector<std::unique_ptr<I> > &&orphans,
              SpeciesCollection<I, F> &&new_species_collection,
              std::vector<I *> &&need_evaluation,
              const std::vector<std::vector<const I*> > &&old_species_individuals
    )
            : orphans(std::move(orphans))
            , new_species_collection(std::move(new_species_collection))
            , need_evaluation(std::move(need_evaluation))
            , old_species_individuals(std::move(old_species_individuals))
    {}
};

}

#endif //SPECIATION_GENUSSEED_H
