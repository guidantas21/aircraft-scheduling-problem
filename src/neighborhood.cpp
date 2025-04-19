#include "ASP.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <utility>


bool ASP::best_improvement_intra_swap(Solution &solution) {
    uint32_t delta = 0; // Delta := improvement in penalty (original penalty - new penalty)
    size_t best_flight_i = 0;
    size_t best_flight_j = 0;
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
        for (size_t flight_i = 0; flight_i < sequence.size() - 1; flight_i++) {

            for (size_t flight_j = flight_i + 1; flight_j < sequence.size(); flight_j++) {
                
                if (sequence[flight_j].get().get_release_time() == sequence[flight_j].get().start_time) {
                    // Then we already now that this is a worse solution
                    continue;
                }
                
                std::swap(sequence[flight_i], sequence[flight_j]);

                penalty = solution.runways[runway_i].prefix_penalty[flight_i];

                if (flight_i == 0) {
                    prev_start_time = sequence[0].get().get_release_time();
                } else {
                    Flight &current_flight = sequence[flight_i].get();
                    Flight &prev_flight = sequence[flight_i - 1].get();

                    prev_start_time = prev_flight.start_time;

                    // prev_start_time becomes current_start_time
                    prev_start_time =
                        std::max(current_flight.get_release_time(),
                                 prev_start_time + prev_flight.get_runway_occupancy_time() +
                                     m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));

                    penalty +=
                            (prev_start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
                }

                // [Fligth_i + 1, Flight_j - 1]
                for (size_t k = flight_i + 1; k < flight_j; k++) {
                    Flight &current_flight = sequence[k].get();
                    Flight &prev_flight = sequence[k - 1].get();

                    // prev_start_time becomes current_start_time
                    prev_start_time =
                        std::max(current_flight.get_release_time(),
                                 prev_start_time + prev_flight.get_runway_occupancy_time() +
                                     m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                    
                    
                    if (current_flight.start_time == prev_start_time) {
                        // Nothing gonna change until fligth_j - 1
                        penalty += solution.runways[runway_i].prefix_penalty[flight_j] - solution.runways[runway_i].prefix_penalty[k];
                        prev_start_time = sequence[flight_j - 1].get().start_time;
                        break;
                    } else {
                        penalty +=
                            (prev_start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
                    }

                    if (penalty >= original_penalty) break;
                }

                // {Flight_j}
                if (penalty >= original_penalty) {
                    std::swap(sequence[flight_i], sequence[flight_j]); // Undo the swap to restore the original sequence
                    continue;
                } else {
                    Flight &current_flight = sequence[flight_j].get();
                    Flight &prev_flight = sequence[flight_j - 1].get();

                    // prev_start_time becomes current_start_time
                    prev_start_time =
                        std::max(current_flight.get_release_time(),
                                 prev_start_time + prev_flight.get_runway_occupancy_time() +
                                     m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));

                    penalty +=
                            (prev_start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
                }


                // [Flight_j + 1, Last]
                for (size_t k = flight_j + 1; k < sequence.size(); k++) {
                    if (penalty >= original_penalty) break;
                    
                    Flight &current_flight = sequence[k].get();
                    Flight &prev_flight = sequence[k - 1].get();

                    // prev_start_time becomes current_start_time
                    prev_start_time =
                        std::max(current_flight.get_release_time(),
                                 prev_start_time + prev_flight.get_runway_occupancy_time() +
                                     m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                    
                    
                    if (current_flight.start_time == prev_start_time) {
                        // Nothing gonna change until end
                        penalty += solution.runways[runway_i].prefix_penalty[sequence.size()] - solution.runways[runway_i].prefix_penalty[k];
                        break;
                    } else {
                        penalty +=
                            (prev_start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
                    }
                }


                if (penalty < original_penalty && original_penalty - penalty > delta) {
                    delta = original_penalty - penalty;
                    best_flight_i = flight_i;
                    best_flight_j = flight_j;
                    best_runway_i = runway_i;
                }

                std::swap(sequence[flight_i], sequence[flight_j]); // Undo the swap to restore the original sequence
            }
        }
    }

    // Apply the best swap found
    if (delta > 0) {
        Runway &best_runway = solution.runways[best_runway_i];

        best_runway.sequence[best_flight_i].get().position = best_flight_j;
        best_runway.sequence[best_flight_j].get().position = best_flight_i;
        std::swap(solution.runways[best_runway_i].sequence[best_flight_i], solution.runways[best_runway_i].sequence[best_flight_j]);

        if (best_flight_i == 0) {
            Flight &current_flight = best_runway.sequence.front().get();
            current_flight.start_time = current_flight.get_release_time();
            best_runway.prefix_penalty[1] = 0;
            best_flight_i++;
        }

        for (size_t flight_i = best_flight_i; flight_i < best_runway.sequence.size(); flight_i++) {
            Flight &current_flight = best_runway.sequence[flight_i].get();
            Flight &prev_flight = best_runway.sequence[flight_i - 1].get();

            current_flight.start_time =
                std::max(current_flight.get_release_time(),
                         prev_flight.start_time + prev_flight.get_runway_occupancy_time() +
                             m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));

            best_runway.prefix_penalty[flight_i + 1] =
                best_runway.prefix_penalty[flight_i] +
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

    uint32_t delta = 0; // Delta := improvement in penalty (original penalty - new penalty)
    uint32_t penalty_i = 0;
    uint32_t penalty_j = 0;
    uint32_t prev_start_time_i = 0;
    uint32_t prev_start_time_j = 0;

    for (size_t runway_i = 0; runway_i < m_instance.get_num_runways() - 1; ++runway_i) {
        for (size_t runway_j = runway_i + 1; runway_j < m_instance.get_num_runways(); ++runway_j) {

            // Get all combinations (flight_i, flight_j)
            for (size_t flight_i = 0; flight_i < solution.runways[runway_i].sequence.size(); ++flight_i) {
                for (size_t flight_j = 0; flight_j < solution.runways[runway_j].sequence.size(); ++flight_j) {

                    uint32_t original_penalty_i = solution.runways[runway_i].penalty;
                    uint32_t original_penalty_j = solution.runways[runway_j].penalty;

                    std::swap(solution.runways[runway_i].sequence[flight_i],
                              solution.runways[runway_j].sequence[flight_j]);

                    penalty_i = solution.runways[runway_i].prefix_penalty[flight_i];
                    penalty_j = solution.runways[runway_j].prefix_penalty[flight_j];

                    // Penalty runway_i
                    if (flight_i == 0) {
                        prev_start_time_i = solution.runways[runway_i].sequence[0].get().get_release_time();
                    } else {
                        Flight &current_flight = solution.runways[runway_i].sequence[flight_i].get();
                        Flight &prev_flight = solution.runways[runway_i].sequence[flight_i - 1].get();

                        prev_start_time_i = prev_flight.start_time;

                        // prev_start_time_i becomes current_start_time_i
                        prev_start_time_i =
                            std::max(current_flight.get_release_time(),
                                     prev_start_time_i + prev_flight.get_runway_occupancy_time() +
                                         m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                        penalty_i += (prev_start_time_i - current_flight.get_release_time()) *
                                     current_flight.get_delay_penalty();
                    }

                    for (size_t k = flight_i + 1; k < solution.runways[runway_i].sequence.size();
                         k++) {
                        if (penalty_i + penalty_j >= original_penalty_i + original_penalty_j) break;

                        Flight &current_flight = solution.runways[runway_i].sequence[k].get();
                        Flight &prev_flight = solution.runways[runway_i].sequence[k - 1].get();

                        // prev_start_time_i becomes current_start_time_i
                        prev_start_time_i =
                            std::max(current_flight.get_release_time(),
                                     prev_start_time_i + prev_flight.get_runway_occupancy_time() +
                                         m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                        
                        if (current_flight.start_time == prev_start_time_i) {
                            // Nothing gonna change until end
                            penalty_i += solution.runways[runway_i].prefix_penalty[solution.runways[runway_i].sequence.size()] - solution.runways[runway_i].prefix_penalty[k];
                            break;
                        } else {
                            penalty_i += (prev_start_time_i - current_flight.get_release_time()) *
                                     current_flight.get_delay_penalty();
                        }
                    }

                    // Penalty runway_j
                    if (flight_j == 0) {
                        prev_start_time_j = solution.runways[runway_j].sequence[0].get().get_release_time();
                    } else {
                        Flight &current_flight = solution.runways[runway_j].sequence[flight_j].get();
                        Flight &prev_flight = solution.runways[runway_j].sequence[flight_j - 1].get();

                        prev_start_time_j = prev_flight.start_time;

                        // prev_start_time_j becomes current_start_time_j
                        prev_start_time_j =
                            std::max(current_flight.get_release_time(),
                                     prev_start_time_j + prev_flight.get_runway_occupancy_time() +
                                         m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                        penalty_j += (prev_start_time_j - current_flight.get_release_time()) *
                                     current_flight.get_delay_penalty();
                    }

                    for (size_t k = flight_j + 1; k < solution.runways[runway_j].sequence.size();
                         k++) {
                        if (penalty_i + penalty_j >= original_penalty_i + original_penalty_j) break;
                        
                        Flight &current_flight = solution.runways[runway_j].sequence[k].get();
                        Flight &prev_flight = solution.runways[runway_j].sequence[k - 1].get();

                        // prev_start_time_j becomes current_start_time_j
                        prev_start_time_j =
                            std::max(current_flight.get_release_time(),
                                     prev_start_time_j + prev_flight.get_runway_occupancy_time() +
                                         m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                        
                        if (current_flight.start_time == prev_start_time_j) {
                            // Nothing gonna change until end
                            penalty_j += solution.runways[runway_j].prefix_penalty[solution.runways[runway_j].sequence.size()] - solution.runways[runway_j].prefix_penalty[k];
                            break;
                        } else {
                            penalty_j += (prev_start_time_j - current_flight.get_release_time()) *
                                     current_flight.get_delay_penalty();
                        }
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

bool ASP::best_improvement_inter_move(Solution &solution) {
    // Previous location best_flight_i in best_runway_i
    //      - If there was a flight in best_flight_i + 1 the move all to the right to --
    // New location best_flight_j in best_runway_j
    //      - If there was a flight at best_flight_j in best_runway_j then move all to teh right to ++ 
    size_t best_flight_i = 0;
    size_t best_flight_j = 0;

    size_t best_runway_i = 0;
    size_t best_runway_j = 0;

    uint32_t delta = 0;
    uint32_t penalty_i = 0;
    uint32_t penalty_j = 0;
    uint32_t prev_start_time_i = 0;
    uint32_t prev_start_time_j = 0;

    for (size_t runway_i = 0; runway_i < m_instance.get_num_runways(); ++runway_i) {
        if (solution.runways[runway_i].sequence.size() == 1) continue; // Prevents a runway to be empty
        for (size_t runway_j = 0; runway_j < m_instance.get_num_runways(); ++runway_j) {
            if (runway_i == runway_j) continue;

            // Get all combinations (flight_i, flight_j)
            for (size_t flight_i = 0; flight_i < solution.runways[runway_i].sequence.size(); ++flight_i) {
                for (size_t flight_j = 0; flight_j < solution.runways[runway_j].sequence.size() + 1; ++flight_j) {

                    // Get the original_penalty of the runways
                    uint32_t original_penalty_i = solution.runways[runway_i].penalty;
                    uint32_t original_penalty_j = solution.runways[runway_j].penalty;

                    // Update runway_i after "remove" flight_i
                    penalty_i = solution.runways[runway_i].prefix_penalty[flight_i];

                    if (flight_i == 0) {
                        prev_start_time_i = solution.runways[runway_i].sequence[1].get().get_release_time();
                    } else {
                        prev_start_time_i = solution.runways[runway_i].sequence[flight_i - 1].get().start_time;

                        // Run this iteration to avoid (k - 1) == flight_i in the for
                        if (flight_i + 1 < solution.runways[runway_i].sequence.size()) {
                            Flight &current_flight = solution.runways[runway_i].sequence[flight_i + 1].get();
                            Flight &prev_flight = solution.runways[runway_i].sequence[flight_i - 1].get();

                            // prev_start_time_i becomes current_start_time_i
                            prev_start_time_i =
                                std::max(current_flight.get_release_time(),
                                        prev_start_time_i + prev_flight.get_runway_occupancy_time() +
                                            m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                            penalty_i += (prev_start_time_i - current_flight.get_release_time()) *
                                        current_flight.get_delay_penalty();                           
                        }
                    }

                    for (size_t k = (flight_i == 0 ? 2 : flight_i + 2); k < solution.runways[runway_i].sequence.size(); k++) {
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

                    // Update runway_j after "add" flight_i from runwway_i at index flight_j
                    penalty_j = solution.runways[runway_j].prefix_penalty[flight_j];

                    if (flight_j == 0) {
                        // !!!!!!!!!!!!!!!!!!! OTIMIZE DPS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                        // se inserir o voo aqui nao mudar o penalty, entao posso skipar todo o for
                        // bastar manter o penalty antigo
                        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                        Flight &current_flight = solution.runways[runway_j].sequence[0].get();
                        Flight &prev_flight = solution.runways[runway_i].sequence[flight_i].get();

                        prev_start_time_j = prev_flight.get_release_time();

                        // prev_start_time_j becomes current_start_time_j
                        prev_start_time_j =
                            std::max(current_flight.get_release_time(),
                                     prev_start_time_j + prev_flight.get_runway_occupancy_time() +
                                         m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                        penalty_j += (prev_start_time_j - current_flight.get_release_time()) *
                                     current_flight.get_delay_penalty(); 
                    } else {
                        Flight &prev_flight = solution.runways[runway_j].sequence[flight_j - 1].get();
                        Flight &current_flight = solution.runways[runway_i].sequence[flight_i].get();

                        prev_start_time_j = prev_flight.start_time;

                        // prev_start_time_j becomes current_start_time_j
                        prev_start_time_j =
                            std::max(current_flight.get_release_time(),
                                     prev_start_time_j + prev_flight.get_runway_occupancy_time() +
                                         m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                        penalty_j += (prev_start_time_j - current_flight.get_release_time()) *
                                     current_flight.get_delay_penalty();

                        if (flight_j < solution.runways[runway_j].sequence.size()) {
                            Flight &current_flight = solution.runways[runway_j].sequence[flight_j].get();
                            Flight &prev_flight = solution.runways[runway_i].sequence[flight_i].get();

                            // prev_start_time_j becomes current_start_time_j
                            prev_start_time_j =
                                std::max(current_flight.get_release_time(),
                                        prev_start_time_j + prev_flight.get_runway_occupancy_time() +
                                            m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));
                            penalty_j += (prev_start_time_j - current_flight.get_release_time()) *
                                        current_flight.get_delay_penalty();
                        } 
                    }

                    for (size_t k = (flight_j == 0 ? 1 : flight_j + 1); k < solution.runways[runway_j].sequence.size(); k++) {
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
                }
            }
        }
    }

    // Apply the best move found
    if (delta > 0) {
        solution.runways[best_runway_i].sequence[best_flight_i].get().position = best_flight_j;
        solution.runways[best_runway_i].sequence[best_flight_i].get().runway = best_runway_j;
        solution.runways[best_runway_i].prefix_penalty.pop_back();
        solution.runways[best_runway_j].prefix_penalty.push_back(0);

        // Add to best_runway_j
        if (best_flight_j == solution.runways[best_runway_j].sequence.size()) {
            solution.runways[best_runway_j].sequence.push_back(solution.runways[best_runway_i].sequence[best_flight_i]);
        } else {
            solution.runways[best_runway_j].sequence.push_back(m_dummy_flight);

            for (size_t k = solution.runways[best_runway_j].sequence.size() - 1; k > best_flight_j; k--) {
                solution.runways[best_runway_j].sequence[k] = solution.runways[best_runway_j].sequence[k - 1];
                solution.runways[best_runway_j].sequence[k].get().position = k;
            }

            solution.runways[best_runway_j].sequence[best_flight_j] = solution.runways[best_runway_i].sequence[best_flight_i];
        }

        // Remove from best_runway_i
        for (size_t k = best_flight_i; k + 1 < solution.runways[best_runway_i].sequence.size(); k++) {
            solution.runways[best_runway_i].sequence[k] = solution.runways[best_runway_i].sequence[k + 1];
            solution.runways[best_runway_i].sequence[k].get().position = k;
        }
        solution.runways[best_runway_i].sequence.pop_back();
        
        // Update prefix best_runway_i
        if (best_flight_i == 0) {
            Flight &current_flight = solution.runways[best_runway_i].sequence[0].get();
            current_flight.start_time = current_flight.get_release_time();
            solution.runways[best_runway_i].prefix_penalty[1] = 0;
            best_flight_i++;
        }

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
