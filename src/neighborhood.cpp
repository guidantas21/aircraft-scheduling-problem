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
    uint32_t delta = 0; // Delta := improvement in penalty (original penalty - new penalty)
    size_t best_i = 0;
    size_t best_j = 0; // Indices of the best swap

    uint32_t original_penalty = solution.runways[runway_i].penalty; // Original penalty of the runway's flight sequence
    std::vector<std::reference_wrapper<Flight>> &sequence =
        solution.runways[runway_i].sequence; // Original sequence of flights on the runway

    // If the penalty is zero or there are fewer than two flights, no swaps are needed
    if (original_penalty == 0 || sequence.size() < 2) {
        return;
    }

    uint32_t penalty = 0;         // Penalty of the new sequence after a swap
    uint32_t prev_start_time = 0; // Tracks the current time during penalty calculation

    // Iterate through all possible pairs of flights in the sequence to evaluate swaps
    for (size_t i = 0; i < sequence.size() - 1; i++) {

        for (size_t j = i + 1; j < sequence.size(); j++) {
            std::swap(sequence[i], sequence[j]);

            penalty = solution.runways[runway_i].prefix_penalty[i];

            if (i == 0) {
                prev_start_time = sequence[0].get().get_release_time();
            } else {
                prev_start_time = sequence[i - 1].get().start_time;
            }

            for (size_t k = (i == 0 ? i + 1 : i); k < sequence.size(); k++) {
                Flight &current_flight = sequence[k].get();
                Flight &prev_flight = sequence[k - 1].get();

                // prev_start_time becomes current_start_time
                prev_start_time =
                    std::max(current_flight.get_release_time(),
                             prev_start_time + prev_flight.get_runway_occupancy_time() +
                                 m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                penalty += (prev_start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
            }

            if (penalty < original_penalty && original_penalty - penalty > delta) {
                delta = original_penalty - penalty;
                best_i = i;
                best_j = j;
            }

            std::swap(sequence[i], sequence[j]); // Undo the swap to restore the original sequence
        }
    }

    // Apply the best swap found
    if (delta > 0) {
        sequence[best_i].get().position = best_j;
        sequence[best_j].get().position = best_i;
        std::swap(solution.runways[runway_i].sequence[best_i], solution.runways[runway_i].sequence[best_j]);

        if (best_i == 0) {
            Flight &current_flight = sequence[0].get();
            current_flight.start_time = current_flight.get_release_time();
            solution.runways[runway_i].prefix_penalty[1] = 0;
            best_i++;
        }

        for (size_t i = best_i; i < sequence.size(); i++) {
            Flight &current_flight = sequence[i].get();
            Flight &prev_flight = sequence[i - 1].get();

            current_flight.start_time =
                std::max(current_flight.get_release_time(),
                         prev_flight.start_time + prev_flight.get_runway_occupancy_time() +
                             m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));

            solution.runways[runway_i].prefix_penalty[i + 1] =
                solution.runways[runway_i].prefix_penalty[i] +
                (current_flight.start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
        }

        solution.runways[runway_i].penalty -= delta;
        solution.objective -= delta;
        assert(solution.test_feasibility(m_instance));
    }
}

void ASP::best_improvement_inter_swap(Solution &solution) {
    size_t best_flight_i = 0;
    size_t best_flight_j = 0;

    size_t best_runway_i = 0;
    size_t best_runway_j = 0;

    uint32_t delta = 0;
    uint32_t penalty_i = 0;
    uint32_t penalty_j = 0;
    uint32_t prev_start_time_i = 0;
    uint32_t prev_start_time_j = 0;

    for (size_t runway_i = 0; runway_i < m_instance.get_num_runways() - 1; ++runway_i) {
        for (size_t runway_j = runway_i + 1; runway_j < m_instance.get_num_runways(); ++runway_j) {

            // Get all combinations (flight_i, flight_j)
            for (size_t flight_i = 0; flight_i < solution.runways[runway_i].sequence.size() - 1; ++flight_i) {
                for (size_t flight_j = 0; flight_j < solution.runways[runway_j].sequence.size(); ++flight_j) {

                    uint32_t original_penalty_i = solution.runways[runway_i].penalty;
                    uint32_t original_penalty_j = solution.runways[runway_j].penalty;

                    std::swap(solution.runways[runway_i].sequence[flight_i],
                              solution.runways[runway_j].sequence[flight_j]);

                    // update runway_i
                    penalty_i = solution.runways[runway_i].prefix_penalty[flight_i];

                    if (flight_i == 0) {
                        prev_start_time_i = solution.runways[runway_i].sequence[0].get().get_release_time();
                    } else {
                        prev_start_time_i = solution.runways[runway_i].sequence[flight_i - 1].get().start_time;
                    }

                    for (size_t k = (flight_i == 0 ? 1 : flight_i); k < solution.runways[runway_i].sequence.size();
                         k++) {
                        Flight &current_flight = solution.runways[runway_i].sequence[k].get();
                        Flight &prev_flight = solution.runways[runway_i].sequence[k - 1].get();

                        // prev_start_time_i becomes current_start_time_i
                        prev_start_time_i =
                            std::max(current_flight.get_release_time(),
                                     prev_start_time_i + prev_flight.get_runway_occupancy_time() +
                                         m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                        penalty_i += (prev_start_time_i - current_flight.get_release_time()) *
                                     current_flight.get_delay_penalty();
                    }

                    // update runway_j
                    penalty_j = solution.runways[runway_j].prefix_penalty[flight_j];

                    if (flight_j == 0) {
                        prev_start_time_j = solution.runways[runway_j].sequence[0].get().get_release_time();
                    } else {
                        prev_start_time_j = solution.runways[runway_j].sequence[flight_j - 1].get().start_time;
                    }

                    for (size_t k = (flight_j == 0 ? 1 : flight_j); k < solution.runways[runway_j].sequence.size();
                         k++) {
                        Flight &current_flight = solution.runways[runway_j].sequence[k].get();
                        Flight &prev_flight = solution.runways[runway_j].sequence[k - 1].get();

                        // prev_start_time_j becomes current_start_time_j
                        prev_start_time_j =
                            std::max(current_flight.get_release_time(),
                                     prev_start_time_j + prev_flight.get_runway_occupancy_time() +
                                         m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                        penalty_j += (prev_start_time_j - current_flight.get_release_time()) *
                                     current_flight.get_delay_penalty();
                    }

                    if (penalty_i + penalty_j < original_penalty_i + original_penalty_j &&
                        original_penalty_i + original_penalty_j - (penalty_i + penalty_j) > delta) {
                        delta = original_penalty_i + original_penalty_j - penalty_i - penalty_j;
                        best_runway_i = runway_i;
                        best_runway_j = runway_j;
                        best_flight_i = flight_i;
                        best_flight_j = flight_j;
                    }

                    // Undo the swap to restore the original sequence
                    std::swap(solution.runways[runway_i].sequence[flight_i],
                              solution.runways[runway_j].sequence[flight_j]);
                }
            }
        }
    }

    // Apply the best swap found
    if (delta > 0) {
        solution.runways[best_runway_i].sequence[best_flight_i].get().position = best_flight_j;
        solution.runways[best_runway_i].sequence[best_flight_i].get().runway = best_runway_j;

        solution.runways[best_runway_j].sequence[best_flight_j].get().position = best_flight_i;
        solution.runways[best_runway_j].sequence[best_flight_j].get().runway = best_runway_i;

        std::swap(solution.runways[best_runway_i].sequence[best_flight_i],
                  solution.runways[best_runway_j].sequence[best_flight_j]);

        // Update prefix best_runway_i
        if (best_flight_i == 0) {
            Flight &current_flight = solution.runways[best_runway_i].sequence[0].get();
            current_flight.start_time = current_flight.get_release_time();
            solution.runways[best_runway_i].prefix_penalty[1] = 0;
            best_flight_i++;
        }

        // why u run away from Sherts?
        // i was scared :(

        for (size_t i = best_flight_i; i < solution.runways[best_runway_i].sequence.size(); i++) {
            Flight &current_flight = solution.runways[best_runway_i].sequence[i].get();
            Flight &prev_flight = solution.runways[best_runway_i].sequence[i - 1].get();

            current_flight.start_time =
                std::max(current_flight.get_release_time(),
                         prev_flight.start_time + prev_flight.get_runway_occupancy_time() +
                             m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));

            solution.runways[best_runway_i].prefix_penalty[i + 1] =
                solution.runways[best_runway_i].prefix_penalty[i] +
                (current_flight.start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
        }

        // Update prefix best_runway_j
        if (best_flight_j == 0) {
            Flight &current_flight = solution.runways[best_runway_j].sequence[0].get();
            current_flight.start_time = current_flight.get_release_time();
            solution.runways[best_runway_j].prefix_penalty[1] = 0;
            best_flight_j++;
        }

        for (size_t j = best_flight_j; j < solution.runways[best_runway_j].sequence.size(); j++) {
            Flight &current_flight = solution.runways[best_runway_j].sequence[j].get();
            Flight &prev_flight = solution.runways[best_runway_j].sequence[j - 1].get();

            current_flight.start_time =
                std::max(current_flight.get_release_time(),
                         prev_flight.start_time + prev_flight.get_runway_occupancy_time() +
                             m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));

            solution.runways[best_runway_j].prefix_penalty[j + 1] =
                solution.runways[best_runway_j].prefix_penalty[j] +
                (current_flight.start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
        }

        // Update penaltys
        solution.runways[best_runway_i].penalty = solution.runways[best_runway_i].prefix_penalty.back();
        solution.runways[best_runway_j].penalty = solution.runways[best_runway_j].prefix_penalty.back();
        solution.objective -= delta;
        assert(solution.test_feasibility(m_instance));
    }
}

void ASP::temp_apply_intra_move(Solution &solution, size_t flight_i, size_t flight_j, size_t runway_i) {
    auto &sequence = solution.runways[runway_i].sequence;
    if (flight_i < flight_j) {
        std::rotate(sequence.begin() + static_cast<long>(flight_i), sequence.begin() + static_cast<long>(flight_i) + 1,
                    sequence.begin() + static_cast<long>(flight_j) + 1);
    }
    if (flight_i > flight_j) {
        std::rotate(sequence.begin() + static_cast<long>(flight_j), sequence.begin() + static_cast<long>(flight_i),
                    sequence.begin() + static_cast<long>(flight_i) + 1);
    }
}

void ASP::best_improvement_intra_move(Solution &solution) {
    size_t best_flight_i = 0;
    size_t best_flight_j = 0;
    size_t best_runway_i = 0;

    int best_delta = 0;

    for (size_t runway_i = 0; runway_i < m_instance.get_num_runways(); ++runway_i) {
        const int sequence_size = static_cast<int>(solution.runways[runway_i].sequence.size());
        const uint32_t original_penalty = static_cast<int>(solution.runways[runway_i].penalty);

        for (int flight_i = 0; flight_i < sequence_size; ++flight_i) {
            // move to posisitions before the current position
            for (int flight_j = flight_i - 1; flight_j >= 0; --flight_j) {
                temp_apply_intra_move(solution, flight_i, flight_j, runway_i);

                // The penalty before the flight j does not change, so we retrieve it from penalty prefix vector
                const uint32_t penalty_before_flight_j = solution.runways[runway_i].prefix_penalty[flight_j];

                uint32_t new_penalty = penalty_before_flight_j;
                uint32_t start_time = 0;
                int initial_k = flight_j;

                if (flight_j == 0) {
                    start_time = solution.runways[runway_i].sequence.front().get().get_release_time();
                    ++initial_k;
                } else {
                    start_time = solution.runways[runway_i].sequence[flight_j - 1].get().start_time;
                }

                for (int flight_k = initial_k; flight_k < sequence_size; ++flight_k) {
                    const Flight &current_flight = solution.runways[runway_i].sequence[flight_k].get();
                    const Flight &prev_flight = solution.runways[runway_i].sequence[flight_k - 1].get();

                    const uint32_t earliest =
                        start_time + prev_flight.get_runway_occupancy_time() +
                        m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id());

                    start_time = std::max(earliest, current_flight.get_release_time());

                    const uint32_t delay = start_time - current_flight.get_release_time();

                    new_penalty += current_flight.get_delay_penalty() * delay;
                }

                const int delta = static_cast<int>(new_penalty) - static_cast<int>(original_penalty);

                assert(solution.runways[runway_i].calculate_total_penalty(m_instance) ==
                       (solution.runways[runway_i].penalty + delta));

                temp_apply_intra_move(solution, flight_j, flight_i, runway_i); // NOLINT

                if (delta < best_delta) {
                    best_delta = delta;
                    best_flight_i = flight_i;
                    best_flight_j = flight_j;
                    best_runway_i = runway_i;
                }
            }

            // move to positions after the current position
            const uint32_t penalty_before_flight_i = solution.runways[runway_i].prefix_penalty[flight_i];

            for (int flight_j = flight_i + 1; flight_j < sequence_size; ++flight_j) {
                temp_apply_intra_move(solution, flight_i, flight_j, runway_i);

                uint32_t new_penalty = penalty_before_flight_i;
                uint32_t start_time = 0;
                int initial_k = flight_i;

                if (flight_i == 0) {
                    start_time = solution.runways[runway_i].sequence.front().get().get_release_time();
                    ++initial_k;
                } else {
                    start_time = solution.runways[runway_i].sequence[flight_i - 1].get().start_time;
                }

                for (int flight_k = initial_k; flight_k < sequence_size; ++flight_k) {
                    const Flight &current_flight = solution.runways[runway_i].sequence[flight_k].get();
                    const Flight &prev_flight = solution.runways[runway_i].sequence[flight_k - 1].get();

                    const uint32_t earliest =
                        start_time + prev_flight.get_runway_occupancy_time() +
                        m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id());

                    start_time = std::max(earliest, current_flight.get_release_time());

                    const uint32_t delay = start_time - current_flight.get_release_time();

                    new_penalty += current_flight.get_delay_penalty() * delay;
                }

                const int delta = static_cast<int>(new_penalty) - static_cast<int>(original_penalty);

                assert(solution.runways[runway_i].calculate_total_penalty(m_instance) ==
                       (solution.runways[runway_i].penalty + delta));

                temp_apply_intra_move(solution, flight_j, flight_i, runway_i); // NOLINT

                if (delta < best_delta) {
                    best_delta = delta;
                    best_flight_i = flight_i;
                    best_flight_j = flight_j;
                    best_runway_i = runway_i;
                }
            }
        }
    }
    if (best_delta < 0) {
        Runway &best_runway = solution.runways[best_runway_i];

        if (best_flight_i < best_flight_j) {
            std::rotate(best_runway.sequence.begin() + static_cast<long>(best_flight_i),
                        best_runway.sequence.begin() + static_cast<long>(best_flight_i) + 1,
                        best_runway.sequence.begin() + static_cast<long>(best_flight_j) + 1);
        }
        if (best_flight_i > best_flight_j) {
            std::rotate(best_runway.sequence.begin() + static_cast<long>(best_flight_j),
                        best_runway.sequence.begin() + static_cast<long>(best_flight_i),
                        best_runway.sequence.begin() + static_cast<long>(best_flight_i) + 1);
        }

        size_t update_initial_i = std::min(best_flight_i, best_flight_j);

        if (update_initial_i == 0) {
            best_runway.sequence.front().get().start_time = best_runway.sequence.front().get().get_release_time();
            best_runway.prefix_penalty[1] = 0;
            ++update_initial_i;
        }

        for (size_t flight_i = update_initial_i; flight_i < best_runway.sequence.size(); ++flight_i) {
            Flight &current_flight = best_runway.sequence[flight_i].get();
            Flight &prev_flight = best_runway.sequence[flight_i - 1].get();

            uint32_t earliest = prev_flight.start_time + prev_flight.get_runway_occupancy_time() +
                                m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id());

            current_flight.start_time = std::max(current_flight.get_release_time(), earliest);

            uint32_t delay = current_flight.start_time - current_flight.get_release_time();
            uint32_t flight_penalty = current_flight.get_delay_penalty() * delay;

            best_runway.prefix_penalty[flight_i + 1] = best_runway.prefix_penalty[flight_i] + flight_penalty;
        }
        best_runway.penalty += best_delta;
        solution.objective += best_delta;
    }
    assert(solution.test_feasibility(m_instance));
}
