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
bool ASP::best_improvement_intra_swap(Solution &solution) {
    uint32_t delta = 0; // Delta := improvement in penalty (original penalty - new penalty)
    size_t best_i = 0;
    size_t best_j = 0; // Indices of the best swap

    size_t best_runway_i = 0;

    // If the penalty is zero or there are fewer than two flights, no swaps are needed
    if (solution.objective == 0) {
        return false;
    }

    uint32_t penalty = 0;         // Penalty of the new sequence after a swap
    uint32_t prev_start_time = 0; // Tracks the current time during penalty calculation

    for (size_t runway_i = 0; runway_i < m_instance.get_num_runways(); ++runway_i) {
        uint32_t original_penalty =
            solution.runways[runway_i].penalty; // Original penalty of the runway's flight sequence
        std::vector<std::reference_wrapper<Flight>> &sequence =
            solution.runways[runway_i].sequence; // Original sequence of flights on the runway

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
                    penalty +=
                        (prev_start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
                }

                if (penalty < original_penalty && original_penalty - penalty > delta) {
                    delta = original_penalty - penalty;
                    best_i = i;
                    best_j = j;
                    best_runway_i = runway_i;
                }

                std::swap(sequence[i], sequence[j]); // Undo the swap to restore the original sequence
            }
        }
    }

    // Apply the best swap found
    if (delta > 0) {
        Runway &best_runway = solution.runways[best_runway_i];

        best_runway.sequence[best_i].get().position = best_j;
        best_runway.sequence[best_j].get().position = best_i;
        std::swap(solution.runways[best_runway_i].sequence[best_i], solution.runways[best_runway_i].sequence[best_j]);

        if (best_i == 0) {
            Flight &current_flight = best_runway.sequence.front().get();
            current_flight.start_time = current_flight.get_release_time();
            best_runway.prefix_penalty[1] = 0;
            best_i++;
        }

        for (size_t i = best_i; i < best_runway.sequence.size(); i++) {
            Flight &current_flight = best_runway.sequence[i].get();
            Flight &prev_flight = best_runway.sequence[i - 1].get();

            current_flight.start_time =
                std::max(current_flight.get_release_time(),
                         prev_flight.start_time + prev_flight.get_runway_occupancy_time() +
                             m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));

            best_runway.prefix_penalty[i + 1] =
                best_runway.prefix_penalty[i] +
                (current_flight.start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
        }

        best_runway.penalty -= delta;
        solution.objective -= delta;
        assert(solution.test_feasibility(m_instance));

        return true;
    }
    return false;
}

bool ASP::best_improvement_inter_swap(Solution &solution) {
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
        return true;
    }
    return false;
}

bool ASP::best_improvement_intra_move(Solution &solution) {
    size_t best_flight_i = 0;
    size_t best_flight_j = 0;
    size_t best_runway_i = 0;

    int best_delta = 0;

    for (size_t runway_i = 0; runway_i < m_instance.get_num_runways(); ++runway_i) {
        const int sequence_size = static_cast<int>(solution.runways[runway_i].sequence.size());
        const uint32_t original_penalty = static_cast<int>(solution.runways[runway_i].penalty);

        for (int flight_i = 0; flight_i < sequence_size; ++flight_i) {
            const Flight &current_flight_i = solution.runways[runway_i].sequence[flight_i].get();

            // Move to posisitions before the flight_i
            for (int flight_j = flight_i - 1; flight_j >= 0; --flight_j) {

                /*
                 * To calculate the penalty
                 * From [(F_0, ..., F_{j-1}), (F_j), (F_{j+1}, ..., F_{i-1}), (F_i), (F_{i+1}, ..., F_{n-1})]
                 * To   [(F_0, ..., F_{j-1}), (F_i), (F_j), (F_{j+1}, ..., F_{i-1}), (F_{i+1}, ..., F_{n-1})]
                 */

                // (F_0, ..., F_{j-1})

                const Flight &current_flight_j = solution.runways[runway_i].sequence[flight_j].get();

                uint32_t new_penalty = solution.runways[runway_i].prefix_penalty[flight_j];

                // (F_0, ..., F_{j-1}) + (F_i)

                uint32_t new_start_time_flight_i = 0;

                if (flight_j == 0) {
                    new_start_time_flight_i = current_flight_i.get_release_time();
                } else {
                    const Flight &prev_flight_j = solution.runways[runway_i].sequence[flight_j - 1].get();

                    const uint32_t earliest_flight_i =
                        prev_flight_j.start_time + prev_flight_j.get_runway_occupancy_time() +
                        m_instance.get_separation_time(prev_flight_j.get_id(), current_flight_i.get_id());

                    new_start_time_flight_i = std::max(earliest_flight_i, current_flight_i.get_release_time());

                    const uint32_t delay_flight_i = new_start_time_flight_i - current_flight_i.get_release_time();

                    new_penalty += current_flight_i.get_delay_penalty() * delay_flight_i;
                }

                // ((F_0, ..., F_{j-1}) + (F_i)) + (F_j)

                const uint32_t earliest_flight_j =
                    new_start_time_flight_i + current_flight_i.get_runway_occupancy_time() +
                    m_instance.get_separation_time(current_flight_i.get_id(), current_flight_j.get_id());

                const uint32_t new_start_time_flight_j =
                    std::max(earliest_flight_j, current_flight_j.get_release_time());

                const uint32_t delay_flight_j = new_start_time_flight_j - current_flight_j.get_release_time();

                new_penalty += current_flight_j.get_delay_penalty() * delay_flight_j;

                // ((F_0, ..., F_{j-1}) + (F_i) + (F_j)) + (F_{j+1}, ..., F_{i-1})

                uint32_t start_time = new_start_time_flight_j;

                for (int flight_k = flight_j + 1; flight_k < flight_i; ++flight_k) {
                    const Flight &current_flight = solution.runways[runway_i].sequence[flight_k].get();
                    const Flight &prev_flight = solution.runways[runway_i].sequence[flight_k - 1].get();

                    const uint32_t earliest =
                        start_time + prev_flight.get_runway_occupancy_time() +
                        m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id());

                    start_time = std::max(earliest, current_flight.get_release_time());

                    const uint32_t delay = start_time - current_flight.get_release_time();

                    new_penalty += current_flight.get_delay_penalty() * delay;
                }

                // ((F_0, ..., F_{j-1}) + (F_i) + (F_j) + (F_{j+1}, ..., F_{i-1})) + (F_{i+1}, ..., F_{n-1})

                uint32_t new_start_time_prev_flight_i = start_time;

                if (flight_i < sequence_size - 1) {

                    // F_{i-1} + F_{i+1}
                    const Flight &prev_flight_i = solution.runways[runway_i].sequence[flight_i - 1].get();
                    const Flight &next_flight_i = solution.runways[runway_i].sequence[flight_i + 1].get();

                    const uint32_t earliest_next_flight_i =
                        new_start_time_prev_flight_i + prev_flight_i.get_runway_occupancy_time() +
                        m_instance.get_separation_time(prev_flight_i.get_id(), next_flight_i.get_id());

                    const uint32_t new_start_time_next_flight_i =
                        std::max(earliest_next_flight_i, next_flight_i.get_release_time());

                    const uint32_t delay = new_start_time_next_flight_i - next_flight_i.get_release_time();

                    new_penalty += next_flight_i.get_delay_penalty() * delay;

                    start_time = new_start_time_next_flight_i;

                    // (F_{i+1}, ..., F_{n-1})
                    for (int flight_k = flight_i + 2; flight_k < sequence_size; ++flight_k) {
                        const Flight &current_flight = solution.runways[runway_i].sequence[flight_k].get();
                        const Flight &prev_flight = solution.runways[runway_i].sequence[flight_k - 1].get();

                        const uint32_t earliest =
                            start_time + prev_flight.get_runway_occupancy_time() +
                            m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id());

                        start_time = std::max(earliest, current_flight.get_release_time());

                        const uint32_t delay = start_time - current_flight.get_release_time();

                        new_penalty += current_flight.get_delay_penalty() * delay;
                    }
                }

                const int delta = static_cast<int>(new_penalty) - static_cast<int>(original_penalty);

                if (delta < best_delta) {
                    best_delta = delta;
                    best_flight_i = flight_i;
                    best_flight_j = flight_j;
                    best_runway_i = runway_i;
                }
            }

            // Move to positions after the after flight_i
            const uint32_t penalty_before_flight_i = solution.runways[runway_i].prefix_penalty[flight_i];

            for (int flight_j = flight_i + 1; flight_j < sequence_size; ++flight_j) {
                /*
                 * From [(F_0, ..., F_{i-1}), (F_i), (F_{i+1}, ..., F_{j-1}), (F_j), (F_{j+1}, ..., F_{n-1})]
                 * To   [(F_0, ..., F_{i-1}), (F_{i+1}, ..., F_{j-1}), (F_j), (F_i), (F_{j+1}, ..., F_{n-1})]
                 */

                const Flight &current_flight_j = solution.runways[runway_i].sequence[flight_j].get();

                // (F_0, ..., F_{i-1})
                uint32_t new_penalty = penalty_before_flight_i;

                // (F_0, ..., F_{i-1}) + (F_{i+1}, ..., F_{j-1}) + (F_j)

                Flight &next_flight_i = solution.runways[runway_i].sequence[flight_i + 1].get();

                uint32_t new_start_time_next_flight_i = 0;

                // F_{i-1} + F_{i+1}
                if (flight_i == 0) {
                    new_start_time_next_flight_i = next_flight_i.get_release_time();
                } else {
                    const Flight &prev_flight_i = solution.runways[runway_i].sequence[flight_i - 1].get();

                    const uint32_t earliest_next_flight_i =
                        prev_flight_i.start_time + prev_flight_i.get_runway_occupancy_time() +
                        m_instance.get_separation_time(prev_flight_i.get_id(), next_flight_i.get_id());

                    new_start_time_next_flight_i = std::max(earliest_next_flight_i, next_flight_i.get_release_time());

                    const uint32_t delay_next_flight_i =
                        new_start_time_next_flight_i - next_flight_i.get_release_time();

                    new_penalty += next_flight_i.get_delay_penalty() * delay_next_flight_i;
                }

                // (F_{i+1}, ..., F_{j-1}) + (F_j)
                uint32_t start_time = new_start_time_next_flight_i;

                for (int flight_k = flight_i + 2; flight_k < flight_j + 1; ++flight_k) {
                    const Flight &current_flight = solution.runways[runway_i].sequence[flight_k].get();
                    const Flight &prev_flight = solution.runways[runway_i].sequence[flight_k - 1].get();

                    const uint32_t earliest =
                        start_time + prev_flight.get_runway_occupancy_time() +
                        m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id());

                    start_time = std::max(earliest, current_flight.get_release_time());

                    const uint32_t delay = start_time - current_flight.get_release_time();

                    new_penalty += current_flight.get_delay_penalty() * delay;
                }

                // (F_0, ..., F_{i-1}) + (F_{i+1}, ..., F_{j-1}) + (F_j) + (F_i)

                uint32_t new_start_time_flight_j = start_time;

                const uint32_t earliest_flight_i =
                    new_start_time_flight_j + current_flight_j.get_runway_occupancy_time() +
                    m_instance.get_separation_time(current_flight_j.get_id(), current_flight_i.get_id());

                const uint32_t new_start_time_flight_i =
                    std::max(earliest_flight_i, current_flight_i.get_release_time());

                const uint32_t delay_flight_i = new_start_time_flight_i - current_flight_i.get_release_time();

                new_penalty += current_flight_i.get_delay_penalty() * delay_flight_i;

                // (F_0, ..., F_{i-1}) + (F_{i+1}, ..., F_{j-1}) + (F_j) + (F_i) + (F_{j+1}, ..., F_{n-1})

                if (flight_j < sequence_size - 1) {
                    Flight &next_flight_j = solution.runways[runway_i].sequence[flight_j + 1].get();

                    const uint32_t earliest_next_flight_j =
                        new_start_time_flight_i + current_flight_i.get_runway_occupancy_time() +
                        m_instance.get_separation_time(current_flight_i.get_id(), next_flight_j.get_id());

                    const uint32_t new_start_time_next_flight_j =
                        std::max(earliest_next_flight_j, next_flight_j.get_release_time());

                    const uint32_t delay_next_flight_j =
                        new_start_time_next_flight_j - next_flight_j.get_release_time();

                    new_penalty += next_flight_j.get_delay_penalty() * delay_next_flight_j;

                    start_time = new_start_time_next_flight_j;

                    for (int flight_k = flight_j + 2; flight_k < sequence_size; ++flight_k) {
                        const Flight &current_flight = solution.runways[runway_i].sequence[flight_k].get();
                        const Flight &prev_flight = solution.runways[runway_i].sequence[flight_k - 1].get();

                        const uint32_t earliest =
                            start_time + prev_flight.get_runway_occupancy_time() +
                            m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id());

                        start_time = std::max(earliest, current_flight.get_release_time());

                        const uint32_t delay = start_time - current_flight.get_release_time();

                        new_penalty += current_flight.get_delay_penalty() * delay;
                    }
                }

                const int delta = static_cast<int>(new_penalty) - static_cast<int>(original_penalty);

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

        assert(solution.test_feasibility(m_instance));

        return true;
    }
    return false;
}

/**
 * @brief Performs an inter-runway reinsertion to find the best improvement in penalty for a given solution.
 *
 * This function evaluates all possible reisertion of all flights within a solution to minimize the penalty
 * associated with the solution. The penalty is calculated based on delays caused by flight scheduling
 * constraints such as release times, runway occupancy times, and separation times.
 *
 */
bool ASP::best_improvement_inter_move(Solution &solution) {
    int best_delta = 0; // Delta := improvement in penalty (new penalty - original penalty)
    std::pair<size_t, size_t> best_o =
        std::make_pair(0, 0); // Which runway and the best flight from it have the best move
    std::pair<size_t, size_t> best_d = std::make_pair(0, 0); // Where put them, the runway and the position, like 0 is
                                                             // the begin, and runway size is the end

    bool improved = false;

    int best_origin_penalty_reduction = 0;
    int best_destiny_penalty_delta = 0;

    int original_penalty = static_cast<int>(solution.objective); // Original penalty of the solution
    int penalty_delta = 0;                                       // Penalty delta of the solution after a move

    // If the penalty is zero or there are fewer than two runways, no move is needed
    if (original_penalty == 0 || solution.runways.size() < 2) {
        return false;
    }

    // Vars associated to origin runway penalty
    size_t origin_new_penalty = 0;       // Penalty of the origin runway without the poped flight
    size_t origin_penalty_reduction = 0; // Penalty associated by the selection of a flight to be poped

    // Vars associated to destiny runway penalty
    size_t destiny_original_runway_penalty = 0; // Penalty of the original sequence of a origin runway
    size_t destiny_new_penalty = 0;             // Penalty of the origin runway without the poped flight
    int destiny_penalty_delta = 0;              // Penalty associated by the selection of a flight to be poped

    uint32_t current_time = 0; // Tracks the current time during penalty calculation

    /*
     * From [(F_0, ..., F_{i-1}), (F_i), (F_{i+1}, ..., F{n-1})]
     *      [(F_0, ..., F_{j-1}), (F_{j}, F_{j+1}, ..., F_{n-1})]
     *
     * To   [(F_0, ..., F_{i-1}), (F_{i+1}, ..., F{n-1})]
     *      [(F_0, ..., F_{j-1}), (F_{i}), (F_{j}, F_{j+1}, ..., F_{n-1})]
     */

    // Iterate through all the runways
    for (size_t runway_o = 0; runway_o < solution.runways.size(); runway_o++) {

        const uint32_t original_runway_i_penalty = solution.runways[runway_o].penalty;

        // Iterate through all the flights of the origin runway
        for (size_t flight_o = 0; flight_o < solution.runways[runway_o].sequence.size(); flight_o++) {

            // calculates the value of penalty reduction

            // [(F_0, ..., F_{i-1}), (F_{i+1}, ..., F_{n-1})]

            // calculates the new penalty
            origin_new_penalty = 0; // reset the value

            // vou ter que mudar e verificar isso dnv dps que tiver com algumas alterações como o prefix_sum
            if (flight_o == 0) { // Se estou retirando o primeiro voo da pista

                // [(F_{i+1}, ..., F_{n-1})]

                current_time = solution.runways[runway_o].sequence[1].get().get_release_time();

                uint32_t release_time = 0;
                uint32_t delay = 0;
                uint32_t earliest_possible = 0;

                for (size_t i = 1; i < solution.runways[runway_o].sequence.size() - 1;
                     i++) { // Não preciso olhar o primeiro
                    earliest_possible =
                        current_time + solution.runways[runway_o].sequence[i].get().get_runway_occupancy_time() +
                        m_instance.get_separation_time(solution.runways[runway_o].sequence[i].get().get_id(),
                                                       solution.runways[runway_o].sequence[i + 1].get().get_id());
                    release_time = solution.runways[runway_o].sequence[i + 1].get().get_release_time();

                    current_time = std::max(release_time, earliest_possible);

                    delay = current_time - release_time;

                    origin_new_penalty += solution.runways[runway_o].sequence[i + 1].get().get_delay_penalty() * delay;
                }
            } else if (flight_o ==
                       solution.runways[runway_o].sequence.size() - 1) { // Se estou retirando o ultimo voo da pista

                // [(F_0, ..., F_{i-1})] (TODO: apply prefix)

                current_time = solution.runways[runway_o].sequence[0].get().get_release_time();

                uint32_t earliest_possible = 0;
                uint32_t release_time = 0;
                uint32_t delay = 0;

                for (size_t i = 0; i < solution.runways[runway_o].sequence.size() - 2;
                     i++) { // Não preciso olhar o último
                    earliest_possible =
                        current_time + solution.runways[runway_o].sequence[i].get().get_runway_occupancy_time() +
                        m_instance.get_separation_time(solution.runways[runway_o].sequence[i].get().get_id(),
                                                       solution.runways[runway_o].sequence[i + 1].get().get_id());
                    release_time = solution.runways[runway_o].sequence[i + 1].get().get_release_time();

                    current_time = std::max(release_time, earliest_possible);

                    delay = current_time - release_time;

                    origin_new_penalty += solution.runways[runway_o].sequence[i + 1].get().get_delay_penalty() * delay;
                }
            } else {

                // [(F_0, ..., F_{i-1}), (F_{i+1}, ..., F_{n-1})]

                /*
                 * TODO:
                 * Compute (F_0, ..., F_{i-1}) with prefix
                 * Compute (F_{i+1}, ..., F_{n-1})
                 */

                current_time = solution.runways[runway_o].sequence[0].get().get_release_time();

                uint32_t earliest_possible = 0;
                uint32_t release_time = 0;
                uint32_t delay = 0;

                for (size_t i = 0; i < solution.runways[runway_o].sequence.size() - 1; i++) {

                    if (i == flight_o - 1) {
                        earliest_possible = current_time +
                                            solution.runways[runway_o].sequence[i].get().get_runway_occupancy_time() +
                                            m_instance.get_separation_time(
                                                solution.runways[runway_o].sequence[i].get().get_id(),
                                                solution.runways[runway_o].sequence[i + 1 + 1].get().get_id());
                        release_time = solution.runways[runway_o].sequence[i + 1 + 1].get().get_release_time();

                        current_time = std::max(release_time, earliest_possible);

                        uint32_t delay = current_time - release_time;

                        origin_new_penalty +=
                            solution.runways[runway_o].sequence[i + 1 + 1].get().get_delay_penalty() * delay;

                        i++; // Increment the i because we dont want to look for the flight_o
                    } else if (i == flight_o + 1) {
                        earliest_possible =
                            current_time +
                            solution.runways[runway_o].sequence[i - 1].get().get_runway_occupancy_time() +
                            m_instance.get_separation_time(solution.runways[runway_o].sequence[i - 1].get().get_id(),
                                                           solution.runways[runway_o].sequence[i + 1].get().get_id());
                        release_time = solution.runways[runway_o].sequence[i + 1].get().get_release_time();

                        current_time = std::max(release_time, earliest_possible);

                        uint32_t delay = current_time - release_time;

                        origin_new_penalty +=
                            solution.runways[runway_o].sequence[i + 1].get().get_delay_penalty() * delay;
                    } else {
                        earliest_possible =
                            current_time + solution.runways[runway_o].sequence[i].get().get_runway_occupancy_time() +
                            m_instance.get_separation_time(solution.runways[runway_o].sequence[i].get().get_id(),
                                                           solution.runways[runway_o].sequence[i + 1].get().get_id());
                        release_time = solution.runways[runway_o].sequence[i + 1].get().get_release_time();

                        current_time = std::max(release_time, earliest_possible);

                        delay = current_time - release_time;

                        origin_new_penalty +=
                            solution.runways[runway_o].sequence[i + 1].get().get_delay_penalty() * delay;
                    }
                }
            }

            origin_penalty_reduction = static_cast<int>(origin_new_penalty) - original_runway_i_penalty;

            Flight &removed_flight = solution.runways[runway_o].sequence[flight_o].get();

            // Iterate through all the runways diferents from the origin
            for (size_t runway_d = 0; (runway_d != runway_o) && (runway_d < solution.runways.size()); runway_d++) {

                // Iterate through all the positions of the destiny runway
                for (size_t position_d = 0; position_d <= solution.runways[runway_d].sequence.size();
                     position_d++) { // Remembering the 0 is begin, the runway.size() is the end

                    // calculates the value of penalty reduction
                    destiny_original_runway_penalty = solution.runways[runway_d].penalty;

                    // calculates the new penalty
                    destiny_new_penalty = 0; // reset the value

                    // [(F_0, ..., F_{j-1}), ()]

                    if (position_d == 0) {

                        current_time = removed_flight.get_release_time();

                        uint32_t earliest_possible = 0;
                        uint32_t release_time = 0;
                        uint32_t delay = 0;

                        earliest_possible =
                            current_time + poped_flight.get_runway_occupancy_time() +
                            m_instance.get_separation_time(poped_flight.get_id(), solution.runways[runway_d].sequence[0].get().get_id());
                        release_time = solution.runways[runway_d].sequence[0].get().get_release_time();

                        current_time = std::max(release_time, earliest_possible);

                        delay = current_time - release_time;

                        destiny_new_penalty += solution.runways[runway_d].sequence[0].get().get_delay_penalty() * delay;     

                        for(size_t i = 0; i < solution.runways[runway_d].sequence.size()-1; i++){
                            earliest_possible =
                                current_time + solution.runways[runway_d].sequence[i].get().get_runway_occupancy_time() +
                                m_instance.get_separation_time(solution.runways[runway_d].sequence[i].get().get_id(), solution.runways[runway_d].sequence[i + 1].get().get_id());
                            release_time = solution.runways[runway_d].sequence[i + 1].get().get_release_time();

                            current_time = std::max(release_time, earliest_possible);

                            delay = current_time - release_time;

                            destiny_new_penalty += solution.runways[runway_d].sequence[i + 1].get().get_delay_penalty() * delay;                          
                        }
                    }else if(position_d == solution.runways[runway_d].sequence.size()){
                        current_time = solution.runways[runway_d].sequence[0].get().get_release_time();

                        uint32_t earliest_possible = 0;
                        uint32_t release_time = 0;
                        uint32_t delay = 0;

                        size_t i;
                        for(i = 0; i < solution.runways[runway_d].sequence.size()-1; i++){

                            earliest_possible =
                                current_time + solution.runways[runway_d].sequence[i].get().get_runway_occupancy_time() +
                                m_instance.get_separation_time(solution.runways[runway_d].sequence[i].get().get_id(), solution.runways[runway_d].sequence[i + 1].get().get_id());
                            release_time = solution.runways[runway_d].sequence[i + 1].get().get_release_time();

                            current_time = std::max(release_time, earliest_possible);

                            delay = current_time - release_time;

                            destiny_new_penalty += solution.runways[runway_d].sequence[i + 1].get().get_delay_penalty() * delay;                            
                        }

                        earliest_possible =
                            current_time + solution.runways[runway_d].sequence[i].get().get_runway_occupancy_time() +
                            m_instance.get_separation_time(solution.runways[runway_d].sequence[i].get().get_id(), poped_flight.get_id());
                        release_time = poped_flight.get_release_time();

                        current_time = std::max(release_time, earliest_possible);

                        delay = current_time - release_time;

                        destiny_new_penalty += removed_flight.get_delay_penalty() * delay;
                    } else {

                        current_time = solution.runways[runway_d].sequence[0].get().get_release_time();

                        uint32_t earliest_possible = 0;
                        uint32_t release_time = 0;
                        uint32_t delay = 0;

                        for(size_t i = 0; i < solution.runways[runway_d].sequence.size()-1; i++){

                            if (i == position_d - 1) {
                                earliest_possible =
                                    current_time + solution.runways[runway_d].sequence[i].get().get_runway_occupancy_time() +
                                    m_instance.get_separation_time(solution.runways[runway_d].sequence[i].get().get_id(), poped_flight.get_id());
                                release_time = poped_flight.get_release_time();

                                current_time = std::max(release_time, earliest_possible);

                                delay = current_time - release_time;

                                destiny_new_penalty += removed_flight.get_delay_penalty() * delay;

                                earliest_possible =
                                    current_time + poped_flight.get_runway_occupancy_time() +
                                    m_instance.get_separation_time(poped_flight.get_id(), solution.runways[runway_d].sequence[i + 1].get().get_id());
                                release_time = solution.runways[runway_d].sequence[i + 1].get().get_release_time();

                                current_time = std::max(release_time, earliest_possible);

                                delay = current_time - release_time;

                                destiny_new_penalty += solution.runways[runway_d].sequence[i + 1].get().get_delay_penalty() * delay;
                            }else{
                                earliest_possible =
                                    current_time + solution.runways[runway_d].sequence[i].get().get_runway_occupancy_time() +
                                    m_instance.get_separation_time(solution.runways[runway_d].sequence[i].get().get_id(), solution.runways[runway_d].sequence[i + 1].get().get_id());
                                release_time = solution.runways[runway_d].sequence[i + 1].get().get_release_time();

                                current_time = std::max(release_time, earliest_possible);

                                delay = current_time - release_time;

                                destiny_new_penalty += solution.runways[runway_d].sequence[i + 1].get().get_delay_penalty() * delay;
                            }
                        }
                    }

                    destiny_penalty_delta =
                        static_cast<int>(destiny_new_penalty) - static_cast<int>(destiny_original_runway_penalty);

                    penalty_delta = origin_penalty_reduction + destiny_penalty_delta;

                    std::cout << "Delta: " << penalty_delta << std::endl;

                    // Check if this move results in the best improvement so far
                    if (penalty_delta < 0 && penalty_delta < best_delta) {
                        improved = true;
                        std::cout << "Melhorou!\n";
                        std::cout << "Pista: " << runway_o+1 << " | Voo " << flight_o << std::endl;
                        std::cout << "Pista: " << runway_d+1 << " | Voo " << position_d << std::endl << std::endl;
                        
                        best_delta = penalty_delta;
                        best_o = std::make_pair(runway_o, flight_o);
                        best_d = std::make_pair(runway_d, position_d);
                        best_origin_penalty_reduction = static_cast<int>(origin_penalty_reduction);
                        best_destiny_penalty_delta = destiny_penalty_delta;
                    }
                }
            }
        }
    }

    if(improved){

        // Apply the best move found
        // Insert the poped flight
        solution.runways[best_d.first].sequence.insert(solution.runways[best_d.first].sequence.begin()+best_d.second, solution.runways[best_o.first].sequence[best_o.second]);
        // solution.runways[best_d.first].penalty += best_destiny_penalty_delta;

        // Erase the poped flight
        for(int i = best_o.second; i < (int)solution.runways[best_o.first].sequence.size()-1; i++){
            solution.runways[best_o.first].sequence[i] = solution.runways[best_o.first].sequence[i+1];
        }solution.runways[best_o.first].sequence.pop_back();
        // solution.runways[best_o.first].penalty += best_origin_penalty_reduction;

        
        // solution.objective += best_delta;

        Solution solution_copy = solution;
        solution_copy.calculate_objective(m_instance);

        

        std::cout << "Old Solution objective: " << solution.objective << std::endl;
        std::cout << "best delta: " << best_delta << std::endl;

        solution.runways[best_d.first].penalty += best_destiny_penalty_delta;
        solution.runways[best_o.first].penalty += best_origin_penalty_reduction;
        solution.objective += best_delta;

        std::cout << "Solution (wrong) runway_o: " << solution.runways[best_o.first].penalty << std::endl;
        std::cout << "Copy solution (right) runway_o: " << solution_copy.runways[best_o.first].penalty << std::endl;
        std::cout << "Solution (wrong) runway_d: " << solution.runways[best_d.first].penalty << std::endl;
        std::cout << "Copy solution (right) runway_d: " << solution_copy.runways[best_d.first].penalty << std::endl;

        std::cout << "Solution (wrong) objective: " << solution.objective << std::endl;
        std::cout << "Copy solution (right) objective: " << solution_copy.objective << std::endl;
        

        assert(solution.test_feasibility(m_instance));
        return true;
    }

    return false;
}
