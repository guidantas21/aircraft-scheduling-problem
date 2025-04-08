#include "ASP.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <utility>

/**
 * @brief Performs an intra-runway swap to find the best improvement in penalty for a given runway.
 *
 * This function evaluates all possible swaps of flights within the sequence of a specific runway
 * to minimize the penalty associated with the sequence. The penalty is calculated based on delays
 * caused by flight scheduling constraints such as release times, runway occupancy times, and separation times.
 *
 */
void ASP::best_improvement_intra_swap(Solution &solution, const size_t runway_i) {
    size_t delta = 0; // Delta := improvement in penalty (original penalty - new penalty)
    size_t best_i = 0;
    size_t best_j = 0; // Indices of the best swap

    uint32_t original_penalty = solution.runways[runway_i].penalty; // Original penalty of the runway's flight sequence
    std::vector<std::reference_wrapper<Flight>> sequence =
        solution.runways[runway_i].sequence; // Original sequence of flights on the runway

    // If the penalty is zero or there are fewer than two flights, no swaps are needed
    if (original_penalty == 0 || sequence.size() < 2) {
        return;
    }

    size_t penalty = 0;        // Penalty of the new sequence after a swap
    uint32_t current_time = 0; // Tracks the current time during penalty calculation

    // Iterate through all possible pairs of flights in the sequence to evaluate swaps
    for (size_t i = 0; i < sequence.size() - 1; i++) {

        for (size_t j = i + 1; j < sequence.size(); j++) {
            std::swap(sequence[i], sequence[j]);

            // Recalculate the penalty for the new sequence after the swap
            penalty = 0;
            current_time = sequence.front().get().get_release_time();

            for (size_t k = 0; k < sequence.size() - 1; k++) {
                Flight &current_flight = sequence[k].get();

                Flight &next_flight = sequence[k + 1].get();

                uint32_t earliest_possible =
                    current_time + current_flight.get_runway_occupancy_time() +
                    m_instance.get_separation_time(sequence[k].get().get_id(), sequence[k + 1].get().get_id());
                uint32_t release_time = next_flight.get_release_time();

                current_time = std::max(release_time, earliest_possible);

                uint32_t delay = current_time - release_time;

                penalty += next_flight.get_delay_penalty() * delay;
            }

            // Check if this swap results in the best improvement so far
            if (penalty < original_penalty && original_penalty - penalty > delta) {
                delta = original_penalty - penalty;
                best_i = i;
                best_j = j;
            }

            std::swap(sequence[i], sequence[j]); // Undo the swap to restore the original sequence
        }
    }
    // Apply the best swap found
    std::swap(solution.runways[runway_i].sequence[best_i], solution.runways[runway_i].sequence[best_j]);
    solution.runways[runway_i].penalty -= delta;
    solution.objective -= delta;

    assert(solution.test_feasibility(m_instance));
}

void ASP::best_improvement_inter_swap(Solution &solution) {
    size_t best_flight_i = 0;
    size_t best_flight_j = 0;

    size_t best_runway_i = 0;
    size_t best_runway_j = 0;

    int best_delta = 0;

    for (size_t runway_i = 0; runway_i < m_instance.get_num_runways(); ++runway_i) {
        for (size_t runway_j = runway_i + 1; runway_j < m_instance.get_num_runways(); ++runway_j) {
            for (size_t flight_i = 0; flight_i < solution.runways[runway_i].sequence.size(); ++flight_i) {
                for (size_t flight_j = 0; flight_j < solution.runways[runway_j].sequence.size(); ++flight_j) {
                    uint32_t objective_before = solution.objective;

                    std::swap(solution.runways[runway_i].sequence[flight_i],
                              solution.runways[runway_j].sequence[flight_j]);

                    solution.update_objective(m_instance);

                    int delta = static_cast<int>(solution.objective - objective_before);

                    std::swap(solution.runways[runway_i].sequence[flight_i],
                              solution.runways[runway_j].sequence[flight_j]);

                    solution.update_objective(m_instance);

                    if (delta < best_delta) {
                        best_delta = delta;

                        best_flight_i = flight_i;
                        best_flight_j = flight_j;

                        best_runway_i = runway_i;
                        best_runway_j = runway_j;
                    }
                }
            }
        }
    }
    if (best_delta < 0) {
        std::swap(solution.runways[best_runway_i].sequence[best_flight_i],
                  solution.runways[best_runway_j].sequence[best_flight_j]);
        solution.update_objective(m_instance);
    }
    assert(solution.test_feasibility(m_instance));
}
