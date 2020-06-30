//
// Created by matteo on 22/6/20.
//

#ifndef SPECIATION_EXCEPTIONS_H
#define SPECIATION_EXCEPTIONS_H

#include <exception>
#include <optional>
#include <string>
#include <sstream>

namespace speciation {

template<typename F>
class invalid_fitness : public std::exception {
    std::optional<F> fitness;
    std::string message;
public:
    invalid_fitness(F value, const std::string &reason = "")
        : invalid_fitness(std::make_optional(value), reason)
    {}

    invalid_fitness(std::optional<F> value, const std::string &reason = "") : fitness(value) {
        std::stringstream _message;
        if (value.has_value()) {
            _message << "Invalid fitness value: " << value.value();
        } else {
            _message << "Invalid fitness, no value present ";
        }

        if (! message.empty()) {
            _message << " (" << reason << ')';
        }

        message = _message.str();
    }

    [[nodiscard]] const char * what() const noexcept override {
        return message.c_str();
    }
};

}

#endif //SPECIATION_EXCEPTIONS_H
