#include "ASP.hpp"
#include "flight.hpp"
#include "runway.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <utility>

void ASP::random_inter_block_swap(Solution &solution) {
    if (solution.runways.size() < 2) {
        return;
    }
    std::uniform_int_distribution<size_t> dist_runway(0, solution.runways.size() - 1);
    size_t runway_i_id = dist_runway(m_generator);
    size_t runway_j_id = dist_runway(m_generator);

    while (runway_i_id == runway_j_id) {
        runway_j_id = dist_runway(m_generator);
    }

    std::uniform_int_distribution<size_t> dist_flight_i(0, solution.runways[runway_i_id].sequence.size() - 1);
    size_t flight_i_pos = dist_flight_i(m_generator);

    size_t flight_j_pos = 0;
    uint32_t best_start_time_delta = std::abs(solution.runways[runway_j_id].sequence[flight_j_pos].get().start_time -
                                              solution.runways[runway_i_id].sequence[flight_i_pos].get().start_time);

    for (size_t flight_pos = 0; flight_pos < solution.runways[runway_j_id].sequence.size(); ++flight_pos) {
        Flight &current_flight = solution.runways[runway_j_id].sequence[flight_pos].get();

        uint32_t start_time_delta =
            std::abs(current_flight.start_time - solution.runways[runway_i_id].sequence[flight_i_pos].get().start_time);

        if (start_time_delta < best_start_time_delta) {
            flight_j_pos = flight_pos;
            best_start_time_delta = start_time_delta;
        }
    }

    std::uniform_int_distribution<size_t> dist_block_i(1, solution.runways[runway_i_id].sequence.size() - flight_i_pos);
    size_t block_i_size = dist_block_i(m_generator);

    std::uniform_int_distribution<size_t> dist_block_j(1, solution.runways[runway_j_id].sequence.size() - flight_j_pos);
    size_t block_j_size = dist_block_j(m_generator);

    if (block_i_size > block_j_size) {
        std::swap(block_i_size, block_j_size);
        std::swap(flight_i_pos, flight_j_pos);
        std::swap(runway_i_id, runway_j_id);
    }

    Runway &runway_i = solution.runways[runway_i_id];
    Runway &runway_j = solution.runways[runway_j_id];

    Flight &flight_i = runway_i.sequence[flight_i_pos].get();
    Flight &flight_j = runway_j.sequence[flight_j_pos].get();

    for (size_t k = 0; k < block_i_size; ++k) {
        std::swap(runway_i.sequence[flight_i_pos + k], runway_j.sequence[flight_j_pos + k]);
    }

    if (flight_i_pos == 0) {
        Flight &current_flight = runway_i.sequence[0].get();
        current_flight.start_time = current_flight.get_release_time();
        runway_i.prefix_penalty[1] = 0;
        flight_i_pos++;
    }

    int original_penalty_runway_i = runway_i.penalty;

    for (size_t i = flight_i_pos; i < runway_i.sequence.size(); ++i) {
        Flight &current_flight = runway_i.sequence[i].get();
        Flight &prev_flight = runway_i.sequence[i - 1].get();

        current_flight.start_time =
            std::max(current_flight.get_release_time(),
                     prev_flight.start_time + prev_flight.get_runway_occupancy_time() +
                         m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));

        runway_i.prefix_penalty[i + 1] =
            runway_i.prefix_penalty[i] +
            (current_flight.start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
    }
    runway_i.penalty = runway_i.prefix_penalty[runway_i.sequence.size()];

    int original_penalty_runway_j = runway_j.penalty;

    if (flight_j_pos == 0) {
        Flight &current_flight = runway_j.sequence[0].get();
        current_flight.start_time = current_flight.get_release_time();
        runway_j.prefix_penalty[1] = 0;
        flight_j_pos++;
    }

    for (size_t j = flight_j_pos; j < runway_j.sequence.size(); ++j) {
        Flight &current_flight = runway_j.sequence[j].get();
        Flight &prev_flight = runway_j.sequence[j - 1].get();

        current_flight.start_time =
            std::max(current_flight.get_release_time(),
                     prev_flight.start_time + prev_flight.get_runway_occupancy_time() +
                         m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));

        runway_j.prefix_penalty[j + 1] =
            runway_j.prefix_penalty[j] +
            (current_flight.start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
    }
    runway_j.penalty = runway_j.prefix_penalty[runway_j.sequence.size()];

    solution.objective += (static_cast<int>(runway_j.penalty) - original_penalty_runway_j) +
                          (static_cast<int>(runway_i.penalty) - original_penalty_runway_i);

    assert(solution.test_feasibility(m_instance));
}

bool ASP::best_improvement_free_space(Solution &solution) {
    size_t best_flight_i = 0;
    size_t best_runway_i = 0;
    size_t best_runway_j = 0;

    uint32_t best_free_space = 0;
    uint32_t free_space = 0;
    uint32_t best_penalty = UINT32_MAX;
    uint32_t penalty = 0;
    uint32_t start_time = 0;

    for (size_t runway_i = 0; runway_i < m_instance.get_num_runways(); ++runway_i) {
        if (solution.runways[runway_i].sequence.size() <= 2) continue;
        std::vector<std::reference_wrapper<Flight>> &sequence = solution.runways[runway_i].sequence; 

        for (size_t flight_i = 1; flight_i < sequence.size() - 1; ++flight_i) {
            if (sequence[flight_i + 1].get().start_time == sequence[flight_i + 1].get().get_release_time()) {
                Flight &next_flight = sequence[flight_i + 1].get();
                Flight &prev_flight = sequence[flight_i - 1].get();

                free_space = next_flight.get_release_time() - (prev_flight.start_time + prev_flight.get_runway_occupancy_time());

                if (free_space > best_free_space) {
                    best_free_space = free_space;
                    best_penalty = UINT32_MAX;
                    
                    Flight &current_flight = sequence[flight_i].get();
                    for (size_t runway_j = 0; runway_j < m_instance.get_num_runways(); ++runway_j) {
                        Flight &last_flight = solution.runways[runway_j].sequence[solution.runways[runway_j].sequence.size() - 1].get();

                        start_time = std::max(current_flight.get_release_time(), last_flight.start_time + last_flight.get_runway_occupancy_time() + m_instance.get_separation_time(last_flight.get_id(), current_flight.get_id()));
                           
                        penalty = (start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();

                        if (penalty < best_penalty) {
                            best_penalty = penalty;
                            best_flight_i = flight_i;
                            best_runway_i = runway_i;
                            best_runway_j = runway_j;
                        }
                    }
                }
            }                
        }
    }

    if (best_runway_i == best_runway_j && best_free_space) {
        uint32_t original_penalty = solution.runways[best_runway_i].penalty;
        std::vector<std::reference_wrapper<Flight>> &sequence = solution.runways[best_runway_i].sequence; 
        Flight &tmp = sequence[best_flight_i].get();

        for (size_t k = best_flight_i; k + 1 < sequence.size(); k++) {
            sequence[k] = sequence[k + 1];
            sequence[k].get().position = k;
        }

        sequence[sequence.size() - 1] = tmp;
        tmp.position = sequence.size() - 1;

        sequence[0].get().start_time = sequence[0].get().get_release_time();
        uint32_t prev_start_time = sequence[0].get().start_time;
        penalty = 0;

        for (size_t k = 1; k < sequence.size(); k++) {
            Flight &current_flight = sequence[k].get();
            Flight &prev_flight = sequence[k - 1].get();

            // prev_start_time becomes current_start_time
            prev_start_time =
                std::max(current_flight.get_release_time(),
                        prev_start_time + prev_flight.get_runway_occupancy_time() +
                            m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id()));

            penalty += (prev_start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();

            current_flight.start_time = prev_start_time;
            solution.runways[best_runway_i].prefix_penalty[k + 1] = penalty; 
        }

        solution.runways[best_runway_i].penalty = penalty;
        if (penalty > original_penalty) solution.objective += penalty - original_penalty;
        else solution.objective -= original_penalty - penalty;
        assert(solution.test_feasibility(m_instance));

        return true;
    } else if (best_free_space) {
        size_t best_flight_j = solution.runways[best_runway_j].sequence.size();
        uint32_t original_penalty_i = solution.runways[best_runway_i].penalty; 
        uint32_t original_penalty_j = solution.runways[best_runway_j].penalty;

        solution.runways[best_runway_i].sequence[best_flight_i].get().position = best_flight_j;
        solution.runways[best_runway_i].sequence[best_flight_i].get().runway = best_runway_j;
        solution.runways[best_runway_i].prefix_penalty.pop_back();
        solution.runways[best_runway_j].prefix_penalty.push_back(0);

        // Add to best_runway_j
        solution.runways[best_runway_j].sequence.push_back(solution.runways[best_runway_i].sequence[best_flight_i]);
        

        // Remove from best_runway_i
        for (size_t k = best_flight_i; k + 1 < solution.runways[best_runway_i].sequence.size(); k++) {
            solution.runways[best_runway_i].sequence[k] = solution.runways[best_runway_i].sequence[k + 1];
            solution.runways[best_runway_i].sequence[k].get().position = k;
        }
        solution.runways[best_runway_i].sequence.pop_back();
        
        // Update prefix best_runway_i
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
        if (solution.runways[best_runway_i].penalty > original_penalty_i) solution.objective += solution.runways[best_runway_i].penalty - original_penalty_i;
        else solution.objective -= original_penalty_i - solution.runways[best_runway_i].penalty;
        if (solution.runways[best_runway_j].penalty > original_penalty_j) solution.objective += solution.runways[best_runway_j].penalty - original_penalty_j;
        else solution.objective -= original_penalty_j - solution.runways[best_runway_j].penalty;
        assert(solution.test_feasibility(m_instance));

        return true;
    }
    return false;
}