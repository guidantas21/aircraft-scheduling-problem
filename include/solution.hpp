#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include <vector>

#include "instance.hpp"

struct Solution {
    std::vector<std::vector<size_t>> schedule;
    size_t objective = 0;

    Solution() = default;

    bool test_feasibility(const Instance &instance) const;
};

#endif
