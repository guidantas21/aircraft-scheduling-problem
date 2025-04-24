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

    std::cout << ">> GILS-RVND\n";

    for (size_t iteration = 1; iteration <= max_iterations; ++iteration) {

        std::cout << "\n[" << iteration << "/" << max_iterations << "]" << '\t';

        std::cout << "Best found: " << best_found.objective << '\n';

        Solution local_best = lowest_release_time_insertion(flights);

        std::cout << "\tInitial solution: " << local_best.objective << '\n';

        RVND(local_best);

        size_t ils_iteration = 1;

        Solution solution = lowest_release_time_insertion(flights_perturbation); // USED AT PERTURBATION
        
        while (ils_iteration <= max_ils_iterations) {
            // solution <= local_best ////////////////////////////////////////
                solution.objective = local_best.objective;

                // Runways
                for (size_t i = 0; i < local_best.runways.size(); i++) {
                    // solution.runways[i] <= local_best.runways[i];
                    solution.runways[i].penalty = local_best.runways[i].penalty;
                    solution.runways[i].prefix_penalty = local_best.runways[i].prefix_penalty;
                    
                    solution.runways[i].sequence.clear();

                    for (size_t j = 0; j < local_best.runways[i].sequence.size(); j++) {
                        // solution.runways[i].sequence[j] <= lcoal_best.runways[i].sequence[j]
                        Flight &flight = local_best.runways[i].sequence[j].get();

                        flights_perturbation[flight.get_id()].start_time = flight.start_time;
                        flights_perturbation[flight.get_id()].runway = flight.runway;
                        flights_perturbation[flight.get_id()].position = flight.position;
                        
                        solution.runways[i].sequence.emplace_back(flights_perturbation[flight.get_id()]);
                    }
                }
            ////////////////////////////////////////////////////////////////////////////

            
            // if (ils_iteration % 5 == 0) std::cout << "ils = " << ils_iteration << '\n';

            auto max_pertubation_iters =
                1 + static_cast<size_t>(std::ceil(alpha * static_cast<double>(m_instance.get_num_runways() / 2)));

            if (ils_iteration > 300) max_pertubation_iters += 3;
            if (ils_iteration > 600) max_pertubation_iters += 3;

            for (size_t perturbation_iteration = 1; perturbation_iteration < max_pertubation_iters;
                 ++perturbation_iteration) {
                    
                if (ils_iteration > 900) P4(solution);
                else random_inter_block_swap(solution);
            }

            RVND(solution);

            if (solution.objective < local_best.objective) {
                std::cout << "ils = " << ils_iteration << '\n';

                // local_best <= solution ////////////////////////////////////////
                    local_best.objective = solution.objective;

                    // Runways
                    for (size_t i = 0; i < solution.runways.size(); i++) {
                        // local_best.runways[i] <= solution.runways[i];
                        local_best.runways[i].penalty = solution.runways[i].penalty;
                        local_best.runways[i].prefix_penalty = solution.runways[i].prefix_penalty;
                        
                        local_best.runways[i].sequence.clear();

                        for (size_t j = 0; j < solution.runways[i].sequence.size(); j++) {
                            // local_best.runways[i].sequence[j] <= solution.runways[i].sequence[j]
                            Flight &flight = solution.runways[i].sequence[j].get();

                            flights[flight.get_id()].start_time = flight.start_time;
                            flights[flight.get_id()].runway = flight.runway;
                            flights[flight.get_id()].position = flight.position;
                            
                            local_best.runways[i].sequence.emplace_back(flights[flight.get_id()]);
                        }
                    }
                ////////////////////////////////////////////////////////////////////////////

                ils_iteration = 0;
            }
            ++ils_iteration;
        }
        std::cout << "\tLocal best solution: " << local_best.objective;
        if (local_best.objective < best_found.objective) {
            best_found = local_best;
            std::cout << "\t(New best solution!)\n\n";

            best_found.print_runway();
            std::cout << "Objective: " << best_found.objective << '\n';
        }
        std::cout << '\n';
    }
    std::cout << "\nBest found: " << best_found.objective << '\n';
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
