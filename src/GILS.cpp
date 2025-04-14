#include "ASP.hpp"
#include <limits>

Solution ASP::GILS_VND(const size_t max_iterations, const size_t max_ils_iterations, const float alpha) { // NOLINT
    Solution best_found;
    best_found.objective = std::numeric_limits<uint32_t>::max();

    for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
        Solution solution = randomized_greedy(alpha);

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
