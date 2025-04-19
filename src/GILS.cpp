#include "ASP.hpp"
#include "flight.hpp"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <omp.h>
#include <sys/types.h>

Solution ASP::parallel_GILS_VND(const size_t max_iterations, const size_t max_ils_iterations, const float alpha) {
    Solution best_found;
    best_found.objective = std::numeric_limits<uint32_t>::max();

#pragma omp parallel
    {
        std::vector<Flight> flights;

        flights.reserve(m_instance.get_num_flights());

        for (size_t i = 0; i < m_instance.get_num_flights(); ++i) {
            flights.emplace_back(i, m_instance.get_release_time(i), m_instance.get_runway_occupancy_time(i),
                                 m_instance.get_delay_penalty(i));
        }

        Solution local_best;
        local_best.objective = std::numeric_limits<uint32_t>::max();

#pragma omp for nowait
        for (size_t iteration = 0; iteration < max_iterations; ++iteration) {

            Solution solution = lowest_release_time_insertion(flights);
            Solution iteration_best = solution;

            size_t ils_iteration = 0;
            while (ils_iteration <= max_ils_iterations) {
                VND(solution);

                if (solution.objective < iteration_best.objective) {
                    iteration_best = solution;
                    ils_iteration = 0;
                }
                ++ils_iteration;
            }

            if (iteration_best.objective < local_best.objective) {
                local_best = iteration_best;
            }
        }

#pragma omp critical
        {
            if (local_best.objective < best_found.objective) {
                best_found = local_best;
            }
        }
    }
    return best_found;
}

Solution ASP::GILS_VND(const size_t max_iterations, const size_t max_ils_iterations, const double alpha) { // NOLINT
    Solution best_found;
    best_found.objective = std::numeric_limits<uint32_t>::max();

    for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
        Solution solution = rand_lowest_release_time_insertion(flights);

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

    for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
        std::cout << iteration << std::endl;

        // Solution solution = lowest_release_time_insertion(flights);
        Solution solution = rand_lowest_release_time_insertion(flights);
        
        Solution local_best = solution;

        RVND(solution);

        size_t ils_iteration = 1;

        while (ils_iteration <= max_ils_iterations) {
            size_t max_pertubation_iters =
                1 + static_cast<size_t>(std::ceil(alpha * static_cast<double>(m_instance.get_num_runways() / 2)));

            for (size_t perturbation_iteration = 0; perturbation_iteration < max_pertubation_iters;
                 ++perturbation_iteration) {

                // if (perturbation_iteration < max_pertubation_iters / 2 ) random_inter_block_swap(solution);
                // else best_improvement_free_space(solution);

                if (false == best_improvement_free_space(solution)) break;
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

Solution ASP::GILS_VND_2(const size_t max_iterations, const size_t max_ils_iterations, const double alpha) { // NOLINT
    Solution best_found;
    best_found.objective = std::numeric_limits<uint32_t>::max();

    for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
        Solution solution = randomized_greedy(alpha, flights);

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
