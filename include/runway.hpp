#ifndef RUNWAY_HPP
#define RUNWAY_HPP

#include <cstdint>
#include <functional>
#include <vector>

#include "flight.hpp"
#include "instance.hpp"

struct Runway {
    std::vector<std::reference_wrapper<Flight>> sequence;
    std::vector<uint32_t> prefix_penalty;
    uint32_t penalty = 0;

    Runway() = default;

    uint32_t calculate_total_penalty(const Instance &instance) const;

    void update_total_penalty(const Instance &instance);

    bool test_sequence_feasibility(const Instance &instance) const;

    bool test_penalty(const Instance &instance) const;

    bool test_feasibility(const Instance &instance) const;

    void print_runway() const;

    void print() const;
};

#endif
