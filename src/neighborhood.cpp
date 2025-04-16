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
    int best_delta = 0; // Delta := improvement in penalty (new penalty - original penalty)
    std::pair<size_t, size_t> best_o = std::make_pair(0,0); // Which runway and the best flight from it have the best move
    std::pair<size_t, size_t> best_d = std::make_pair(0,0); // Where put them, the runway and the position, like 0 is the 
                                                            // begin, and runway size is the end

    bool improved = false;

    int best_origin_penalty_reduction = 0;
    int best_destiny_penalty_delta = 0;


    int original_penalty = solution.objective; // Original penalty of the solution
    int penalty_delta = 0;                       // Penalty delta of the solution after a move

    // If the penalty is zero or there are fewer than two runways, no move is needed
    if (original_penalty == 0 || solution.runways.size() < 2) {
        return;
    }

    // Vars associated to origin runway penalty
    size_t origin_original_runway_penalty = 0;  // Penalty of the original sequence of a origin runway
    size_t origin_new_penalty = 0;              // Penalty of the origin runway without the poped flight
    int origin_penalty_reduction = 0;           // Penalty associated by the selection of a flight to be poped

    // Vars associated to destiny runway penalty
    size_t destiny_original_runway_penalty = 0; // Penalty of the original sequence of a origin runway
    size_t destiny_new_penalty = 0;             // Penalty of the origin runway without the poped flight
    int destiny_penalty_delta = 0;              // Penalty associated by the selection of a flight to be poped

    uint32_t current_time = 0;          // Tracks the current time during penalty calculation

    // Iterate through all the runways
    for (size_t runway_o = 0; runway_o < solution.runways.size(); runway_o++){

        // Iterate through all the flights of the origin runway
        for (size_t flight_o = 0; flight_o < solution.runways[runway_o].sequence.size(); flight_o++){

            //calculates the value of penalty reduction
            origin_original_runway_penalty = solution.runways[runway_o].penalty;
            

            //calculates the new penalty
            origin_new_penalty = 0; //reset the value

            //vou ter que mudar e verificar isso dnv dps que tiver com algumas alterações como o prefix_sum
            if(flight_o == 0){ // Se estou retirando o primeiro voo da pista
                
                current_time = solution.runways[runway_o].sequence[1].get().get_release_time();

                uint32_t earliest_possible;
                uint32_t release_time;
                uint32_t delay;

                for(size_t i = 1; i < solution.runways[runway_o].sequence.size()-1; i++){ // Não preciso olhar o primeiro
                    earliest_possible =
                        current_time + solution.runways[runway_o].sequence[i].get().get_runway_occupancy_time() +
                        m_instance.get_separation_time(solution.runways[runway_o].sequence[i].get().get_id(), solution.runways[runway_o].sequence[i + 1].get().get_id());
                    release_time = solution.runways[runway_o].sequence[i + 1].get().get_release_time();

                    current_time = std::max(release_time, earliest_possible);

                    delay = current_time - release_time;

                    origin_new_penalty += solution.runways[runway_o].sequence[i + 1].get().get_delay_penalty() * delay;
                }
            }else if(flight_o == solution.runways[runway_o].sequence.size()-1){ // Se estou retirando o ultimo voo da pista

                current_time = solution.runways[runway_o].sequence[0].get().get_release_time();

                uint32_t earliest_possible;
                uint32_t release_time;
                uint32_t delay;

                for(size_t i = 0; i < solution.runways[runway_o].sequence.size()-2; i++){ // Não preciso olhar o último
                    earliest_possible =
                        current_time + solution.runways[runway_o].sequence[i].get().get_runway_occupancy_time() +
                        m_instance.get_separation_time(solution.runways[runway_o].sequence[i].get().get_id(), solution.runways[runway_o].sequence[i + 1].get().get_id());
                    release_time = solution.runways[runway_o].sequence[i + 1].get().get_release_time();

                    current_time = std::max(release_time, earliest_possible);

                    delay = current_time - release_time;

                    origin_new_penalty += solution.runways[runway_o].sequence[i + 1].get().get_delay_penalty() * delay;
                }
            }else{

                current_time = solution.runways[runway_o].sequence[0].get().get_release_time();

                uint32_t earliest_possible;
                uint32_t release_time;
                uint32_t delay;

                for(size_t i = 0; i < solution.runways[runway_o].sequence.size()-1; i++){

                    if(i == flight_o-1){
                        earliest_possible =
                            current_time + solution.runways[runway_o].sequence[i].get().get_runway_occupancy_time() +
                            m_instance.get_separation_time(solution.runways[runway_o].sequence[i].get().get_id(), solution.runways[runway_o].sequence[i + 1 + 1].get().get_id());
                        release_time = solution.runways[runway_o].sequence[i + 1 + 1].get().get_release_time();

                        current_time = std::max(release_time, earliest_possible);

                        uint32_t delay = current_time - release_time;

                        origin_new_penalty += solution.runways[runway_o].sequence[i + 1 + 1].get().get_delay_penalty() * delay;

                        i++; // Increment the i because we dont want to look for the flight_o
                    }else if(i == flight_o+1){
                        earliest_possible =
                            current_time + solution.runways[runway_o].sequence[i-1].get().get_runway_occupancy_time() +
                            m_instance.get_separation_time(solution.runways[runway_o].sequence[i-1].get().get_id(), solution.runways[runway_o].sequence[i + 1].get().get_id());
                        release_time = solution.runways[runway_o].sequence[i + 1].get().get_release_time();

                        current_time = std::max(release_time, earliest_possible);

                        uint32_t delay = current_time - release_time;

                        origin_new_penalty += solution.runways[runway_o].sequence[i + 1].get().get_delay_penalty() * delay;
                    }else{
                        earliest_possible =
                            current_time + solution.runways[runway_o].sequence[i].get().get_runway_occupancy_time() +
                            m_instance.get_separation_time(solution.runways[runway_o].sequence[i].get().get_id(), solution.runways[runway_o].sequence[i + 1].get().get_id());
                        release_time = solution.runways[runway_o].sequence[i + 1].get().get_release_time();

                        current_time = std::max(release_time, earliest_possible);

                        delay = current_time - release_time;

                        origin_new_penalty += solution.runways[runway_o].sequence[i + 1].get().get_delay_penalty() * delay;
                    }

                }
            }

            origin_penalty_reduction = origin_new_penalty - origin_original_runway_penalty;

            Flight& poped_flight = solution.runways[runway_o].sequence[flight_o].get();

            // Iterate through all the runways diferents from the origin
            for (size_t runway_d = 0; (runway_d != runway_o) && (runway_d < solution.runways.size()); runway_d++){

                // Iterate through all the positions of the destiny runway
                for (size_t position_d = 0; position_d <= solution.runways[runway_d].sequence.size(); position_d++){ // Remembering the 0 is begin, the runway.size() is the end

                    //calculates the value of penalty reduction
                    destiny_original_runway_penalty = solution.runways[runway_d].penalty;
                    

                    //calculates the new penalty
                    destiny_new_penalty = 0; //reset the value

                    if(position_d == 0){

                        current_time = poped_flight.get_release_time();
                        
                        uint32_t earliest_possible;
                        uint32_t release_time;
                        uint32_t delay;

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

                        uint32_t earliest_possible;
                        uint32_t release_time;
                        uint32_t delay;

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

                        destiny_new_penalty += poped_flight.get_delay_penalty() * delay;
                    }else{

                        current_time = solution.runways[runway_d].sequence[0].get().get_release_time();

                        uint32_t earliest_possible;
                        uint32_t release_time;
                        uint32_t delay;

                        for(size_t i = 0; i < solution.runways[runway_d].sequence.size()-1; i++){

                            if(i == position_d-1){
                                earliest_possible =
                                    current_time + solution.runways[runway_d].sequence[i].get().get_runway_occupancy_time() +
                                    m_instance.get_separation_time(solution.runways[runway_d].sequence[i].get().get_id(), poped_flight.get_id());
                                release_time = poped_flight.get_release_time();

                                current_time = std::max(release_time, earliest_possible);

                                delay = current_time - release_time;

                                destiny_new_penalty += poped_flight.get_delay_penalty() * delay;

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

                    destiny_penalty_delta = destiny_new_penalty - destiny_original_runway_penalty;

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
                        best_origin_penalty_reduction = origin_penalty_reduction;
                        best_destiny_penalty_delta = destiny_penalty_delta;
                    }
                }
            }
        }
    }

    if(improved){
        std::cout << "Final:\n";
        std::cout << "Pista: " << best_o.first+1 << " | Voo " << best_o.second << " | Delta: " << best_origin_penalty_reduction << std::endl;
        std::cout << "Pista: " << best_d.first+1 << " | Voo " << best_d.second << " | Delta: " << best_destiny_penalty_delta << std::endl << std::endl;


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
    }
}