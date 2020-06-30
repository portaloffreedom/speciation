//
// Created by matteo on 6/12/20.
//

#ifndef SPECIATION_AGE_H
#define SPECIATION_AGE_H

namespace speciation {

class Age {
    /// Age of the species (in generations)
    unsigned int _generations;
    /// Age of the species (in evaluations)
    unsigned int _evaluations;
    /// Number of generations in which the Species saw no improvements.
    unsigned int _no_improvements;

public:
    Age()
            : _generations(0)
            , _evaluations(0)
            , _no_improvements(0)
    {}

    // Getters
    [[nodiscard]] unsigned int generations() const { return _generations; }
    [[nodiscard]] unsigned int evaluations() const { return _evaluations; }
    [[nodiscard]] unsigned int no_improvements() const { return _no_improvements; }

    // Increasers
    void increase_generations() { _generations++; }
    void increase_evaluations() { _evaluations++; }
    void increase_no_improvements() { _no_improvements++; }

    // Resetters
    /**
     * Makes the age young again
     */
    void reset_generations() {
        _generations = 0;
        _no_improvements = 0;
    }

    void reset_no_improvements() {
        _no_improvements = 0;
    }

    void reset_evaluations() {
        _evaluations = 0;
    }

    // Operators override
    bool operator== (const Age &other) const
    {
        return _generations == other._generations
               && _evaluations == other._evaluations
               && _no_improvements == other._no_improvements;
    }

    bool operator!= (const Age &other) const
    { return ! (*this == other); }

    // TODO serialize
    // TODO deserialize
};

}


#endif //SPECIATION_AGE_H
