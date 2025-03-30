#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include <vector>

#include "flight.hpp"
#include "instance.hpp"
#include "runway.hpp"

struct Solution {
    std::vector<Runway> runways;
    size_t objective = 0;

    Solution() = default;

    Solution(const Instance &instance);

    bool test_feasibility(const Instance &instance, std::vector<Flight> &flights) const;
};

#endif
