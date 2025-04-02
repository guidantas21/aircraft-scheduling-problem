#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include <cstdint>
#include <vector>

#include "flight.hpp"
#include "instance.hpp"
#include "runway.hpp"

struct Solution {
    std::vector<Runway> runways;
    size_t objective = 0;

    Solution() = default;

    Solution(const Instance &instance);

    uint32_t calculate_objective(const Instance &instance, const std::vector<Flight> &flights) const;

    void update_objective(const Instance &instance, const std::vector<Flight> &flights);

    bool test_feasibility(const Instance &instance, const std::vector<Flight> &flights) const;

    void print() const;
};

#endif
