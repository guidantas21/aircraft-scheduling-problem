#include "ASP.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>

Solution ASP::GRASP_VND(const size_t max_iterations) {
    Solution best_solution;
    best_solution.objective = std::numeric_limits<uint32_t>::max();

    for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
        Solution initial_solution = randomized_greedy(0.01);

        Solution vnd_solution = VND(initial_solution);

        if (vnd_solution.objective < best_solution.objective) {
            best_solution = vnd_solution;
        }
    }
    assert(best_solution.test_feasibility(m_instance));

    return best_solution;
}
