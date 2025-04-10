#include "ASP.hpp"

#include <cassert>
#include <functional>
#include <iostream>

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

/**
 * @brief Performs an inter-runway reinsertion to find the best improvement in penalty for a given solution.
 *
 * This function evaluates all possible reisertion of all flights within a solution to minimize the penalty
 * associated with the solution. The penalty is calculated based on delays caused by flight scheduling
 * constraints such as release times, runway occupancy times, and separation times.
 *
 */
void ASP::best_improvement_inter_move(Solution &solution) {
    size_t delta = 0; // Delta := improvement in penalty (original penalty - new penalty)
    std::pair<size_t, size_t> best_o = std::make_pair(0,0); // Which runway and the best flight from it have the best move
    std::pair<size_t, size_t> best_d = std::make_pair(0,0); // Where put them, the runway and the position, like 0 is the 
                                                            // begin, and runway size is the end

    uint32_t original_penalty = solution.objective; // Original penalty of the solution

    // If the penalty is zero or there are fewer than two runways, no move is needed
    if (original_penalty == 0 || solution.runways.size() < 2) {
        return;
    }

    size_t penalty = 0;                 // Penalty of the new sequence after a swap
    size_t original_runway_penalty = 0; // Penalty of the original sequence of a origin runway
    size_t penalty_reduction = 0;       // Penalty associated by the selection of a flight
    uint32_t current_time = 0;          // Tracks the current time during penalty calculation

    // Iterate through all the runways
    for (size_t runway_o = 0; runway_o < solution.runways.size(); runway_o++){ 

        // Iterate through all the flights of the origin runway
        for (size_t flight_o = 0; flight_o < solution.runways[runway_o].sequence.size(); flight_o++){
            std::cout << "Voo [" << solution.runways[runway_o].sequence[flight_o].get().get_id()+1 << "] retirado da pista [" << runway_o+1 << "]\n";

            // When I select a flight to move I could calculate the penalty reduction associated from it
            // Pop the flight and update penalty_reduction
            original_runway_penalty = solution.runways[runway_o].penalty;
            Flight& poped_flight = solution.runways[runway_o].sequence[flight_o].get(); // Stores the poped flight to reinsert it later
            solution.runways[runway_o].sequence.erase(solution.runways[runway_o].sequence.begin()+flight_o); // pop the flight

            solution.runways[runway_o].update_total_penalty(m_instance); // udpate the runway penalty after

            solution.print();

            std::cout << "Old penalty: " << original_runway_penalty << std::endl;
            std::cout << "New penalty: " << solution.runways[runway_o].penalty << std::endl;

            penalty_reduction = original_runway_penalty - solution.runways[runway_o].penalty; // update the penalty_reduction

            // // Iterate through all the runways diferents from the origin
            // for (size_t runway_d = 0; (runway_d != runway_o) && (runway_d < sequence.size()); runway_d++){

            //     // Iterate through all the positions of the destiny runway
            //     for (size_t position_d = 0; position_d <= sequence[runway_d].size(); position_d++){ // Remembering the 0 is begin, the runway.size() is the end

            //         // Check if this swap results in the best improvement so far
            //         if (penalty < original_penalty && original_penalty - penalty > delta) {
            //             delta = original_penalty - penalty;
            //             best_o = std::make_pair(runway_o, flight_o);
            //             best_d = std::make_pair(runway_d, position_d);
            //         }
            //     }
            // }

            // Undo the pop
            solution.runways[runway_o].sequence.insert(solution.runways[runway_o].sequence.begin()+flight_o, poped_flight);

            solution.print();
        }
    }

    // // Iterate through all possible pairs of flights in the sequence to evaluate swaps
    // for (size_t i = 0; i < sequence.size() - 1; i++) {

    //     for (size_t j = i + 1; j < sequence.size(); j++) {
    //         std::swap(sequence[i], sequence[j]);

    //         // Recalculate the penalty for the new sequence after the swap
    //         penalty = 0;
    //         current_time = sequence.front().get().get_release_time();

    //         for (size_t k = 0; k < sequence.size() - 1; k++) {
    //             Flight &current_flight = sequence[k].get();

    //             Flight &next_flight = sequence[k + 1].get();

    //             uint32_t earliest_possible =
    //                 current_time + current_flight.get_runway_occupancy_time() +
    //                 m_instance.get_separation_time(sequence[k].get().get_id(), sequence[k + 1].get().get_id());
    //             uint32_t release_time = next_flight.get_release_time();

    //             current_time = std::max(release_time, earliest_possible);

    //             uint32_t delay = current_time - release_time;

    //             penalty += next_flight.get_delay_penalty() * delay;
    //         }

    //         // Check if this swap results in the best improvement so far
    //         if (penalty < original_penalty && original_penalty - penalty > delta) {
    //             delta = original_penalty - penalty;
    //             best_i = i;
    //             best_j = j;
    //         }

    //         std::swap(sequence[i], sequence[j]); // Undo the swap to restore the original sequence
    //     }
    // }

    // // Apply the best swap found
    // std::swap(solution.runways[runway_i].sequence[best_i], solution.runways[runway_i].sequence[best_j]);
    // solution.runways[runway_i].penalty -= delta;
    // solution.objective -= delta;

    // assert(solution.test_feasibility(m_instance));
}