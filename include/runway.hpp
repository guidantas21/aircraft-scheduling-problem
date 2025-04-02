#ifndef RUNWAY_HPP
#define RUNWAY_HPP

#include <cstdint>
#include <vector>

#include "flight.hpp"
#include "instance.hpp"

struct Runway {
    std::vector<size_t> sequence;
    uint32_t penalty = 0;

    Runway() = default;

    Runway(std::vector<size_t> &sequence, size_t penalty);

    uint32_t calculate_total_penalty(const Instance &instance, const std::vector<Flight> &flights) const;

    void update_total_penalty(const Instance &instance, const std::vector<Flight> &flights);

    bool test_sequence_feasibility(const Instance &instance) const;

    bool test_penalty(const Instance &instance, const std::vector<Flight> &flights) const;

    bool test_feasibility(const Instance &instance, const std::vector<Flight> &flights) const;

    void print() const;
};

#endif
