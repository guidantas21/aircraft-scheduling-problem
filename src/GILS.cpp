#include "ASP.hpp"

#include <cassert>
#include <iostream>
#include <limits>
#include <omp.h>

thread_local std::mt19937 ASP::m_generator = []() {
    std::random_device rd;
    std::seed_seq seed{rd(), rd(),
                       static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()),
                       static_cast<unsigned int>(omp_get_thread_num())};
    return std::mt19937(seed);
}();

Solution ASP::GILS_VND(const size_t max_iterations, const size_t max_ils_iterations, const float alpha) { // NOLINT
    Solution best_found;
    best_found.objective = std::numeric_limits<uint32_t>::max();

    for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
        Solution solution = randomized_greedy(alpha);

        assert(initial_solution.test_feasibility(m_instance));

        Solution local_best = solution;

        size_t ils_iteration = 0;
        while (ils_iteration <= max_ils_iterations) {
            VND(solution);

            if (solution.objective < local_best.objective) {
                local_best = solution;
                ils_iteration = 0;
            }
            ++ils_iteration;
        }

        if (local_best.objective < best_found.objective) {
            best_found = local_best;
        }
    }
    return best_found;
}
