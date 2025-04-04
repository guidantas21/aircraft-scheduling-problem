#include "moves.hpp"

/**
 * @brief Performs an intra-runway swap to find the best improvement in penalty for a given runway.
 * 
 * This function evaluates all possible swaps of flights within the sequence of a specific runway
 * to minimize the penalty associated with the sequence. The penalty is calculated based on delays
 * caused by flight scheduling constraints such as release times, runway occupancy times, and separation times.
 * 
 */
void moves::best_improvement_intra_swap(const Instance &instance, Solution &solution, const size_t runway_i, const std::vector<Flight> &flights) {
    size_t delta = 0;                                                     // Delta := improvement in penalty (original penalty - new penalty)
    size_t best_i = 0, best_j = 0;                                                // Indices of the best swap

    uint32_t original_penalty = solution.runways[runway_i].penalty;       // Original penalty of the runway's flight sequence
    std::vector<size_t> sequence = solution.runways[runway_i].sequence;   // Original sequence of flights on the runway

    // If the penalty is zero or there are fewer than two flights, no swaps are needed
    if (original_penalty == 0 || sequence.size() < 2) return;             
    
    size_t penalty;                                                       // Penalty of the new sequence after a swap
    uint32_t current_time;                                                // Tracks the current time during penalty calculation

    // Iterate through all possible pairs of flights in the sequence to evaluate swaps
    for (size_t i = 0; i < sequence.size() - 1; i++) {

        for (size_t j = i + 1; j < sequence.size(); j++) {
            std::swap(sequence[i], sequence[j]);

            // Recalculate the penalty for the new sequence after the swap
            penalty = 0;
            current_time = flights[sequence.front()].get_release_time();

            for (size_t k = 0; k < sequence.size() - 1; k++) {
                Flight current_flight = flights[sequence[k]];

                Flight next_flight = flights[sequence[k + 1]];

                uint32_t earliest_possible = current_time + current_flight.get_runway_occupancy_time() +
                                            instance.get_separation_time(sequence[k], sequence[k + 1]);
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

            std::swap(sequence[i], sequence[j]);                           // Undo the swap to restore the original sequence
        }
    }
    
    // Apply the best swap found
    std::swap(solution.runways[runway_i].sequence[best_i], solution.runways[runway_i].sequence[best_j]);
    solution.runways[runway_i].penalty -= delta;
    solution.objective -= delta;
}
    