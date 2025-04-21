#include "ASP.hpp"

#include <cassert>
#include <cstddef>
#include <limits>
#include <omp.h>
#include <sys/types.h>

Solution ASP::GILS_VND(const size_t max_iterations, const size_t max_ils_iterations, const double alpha) { // NOLINT
    Solution best_found;
    best_found.objective = std::numeric_limits<uint32_t>::max();

    for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
        Solution solution = lowest_release_time_insertion();

        Solution local_best = solution;

        VND(solution);

        size_t ils_iteration = 1;

        while (ils_iteration <= max_ils_iterations) {
            size_t max_pertubation_iters =
                1 + static_cast<size_t>(std::ceil(alpha * static_cast<double>(m_instance.get_num_runways() / 2)));

            for (size_t perturbation_iteration = 0; perturbation_iteration < max_pertubation_iters;
                 ++perturbation_iteration) {

                random_inter_block_swap(solution);
            }
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

Solution ASP::GILS_RVND(const size_t max_iterations, const size_t max_ils_iterations, const double alpha) { // NOLINT
    Solution best_found;
    best_found.objective = std::numeric_limits<uint32_t>::max();

    for (size_t iteration = 1; iteration <= max_iterations; ++iteration) {
        Solution solution = lowest_release_time_insertion();

        Solution local_best = solution;

        RVND(solution);

        size_t ils_iteration = 1;

        while (ils_iteration <= max_ils_iterations) {
            auto max_pertubation_iters =
                1 + static_cast<size_t>(std::ceil(alpha * static_cast<double>(m_instance.get_num_runways() / 2)));

            for (size_t perturbation_iteration = 1; perturbation_iteration < max_pertubation_iters;
                 ++perturbation_iteration) {

                if (not best_improvement_free_space(solution)) {
                    random_inter_block_swap(solution);
                    break;
                }
            }
            RVND(solution);

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
