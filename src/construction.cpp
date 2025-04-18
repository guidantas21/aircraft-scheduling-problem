#include "ASP.hpp"

#include "flight.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>

struct Insertion {
    size_t candidate_i;
    uint32_t start_time;
    uint32_t penalty;
    uint32_t runway;

    Insertion(size_t candidate_i, uint32_t start_time, uint32_t penalty, uint32_t runway)
        : candidate_i(candidate_i), start_time(start_time), penalty(penalty), runway(runway) {}
};

Solution ASP::randomized_greedy(const double alpha, std::vector<Flight> &flights) {
    Solution solution(m_instance);

    std::vector<std::reference_wrapper<Flight>> candidate_list;
    candidate_list.reserve(m_instance.get_num_flights());

    for (size_t i = 0; i < m_instance.get_num_flights(); ++i) {
        candidate_list.emplace_back(flights[i]);
    }
    std::vector<size_t> candidates_position(m_instance.get_num_flights());

    std::sort(candidate_list.begin(), candidate_list.end(), [](const auto flight_a, const auto flight_b) {
        return flight_a.get().get_release_time() > flight_b.get().get_release_time();
    });

    for (size_t runway_i = 0; runway_i < m_instance.get_num_runways(); ++runway_i) {
        solution.runways[runway_i].sequence.emplace_back(candidate_list.back());
        solution.runways[runway_i].prefix_penalty.push_back(0);

        Flight &candidate = candidate_list.back().get();

        candidate.position = 0;
        candidate.runway = runway_i;
        candidate.start_time = candidate.get_release_time();

        candidate_list.pop_back();
    }

    while (not candidate_list.empty()) {
        std::vector<Insertion> possible_insertions;

        possible_insertions.reserve(m_instance.get_num_runways() * candidate_list.size());

        for (int candidate_i = static_cast<int>(candidate_list.size()) - 1; candidate_i >= 0; --candidate_i) {
            Flight &candidate = candidate_list[candidate_i];

            candidates_position[candidate.get_id()] = candidate_i;

            for (size_t runway_i = 0; runway_i < solution.runways.size(); ++runway_i) {
                auto last_flight = solution.runways[runway_i].sequence.back();

                uint32_t earliest = last_flight.get().start_time + last_flight.get().get_runway_occupancy_time() +
                                    m_instance.get_separation_time(last_flight.get().get_id(), candidate.get_id());

                uint32_t start_time = std::max(earliest, candidate.get_release_time());

                uint32_t delay = start_time - candidate.get_release_time();

                size_t insertion_penalty = candidate.get_delay_penalty() * delay;

                possible_insertions.emplace_back(candidate_i, start_time, insertion_penalty, runway_i);
            }
        }
        std::sort(possible_insertions.begin(), possible_insertions.end(),
                  [](const Insertion &a, const Insertion &b) { return a.penalty < b.penalty; });

        std::uniform_int_distribution<size_t> dist_selection(
            0, std::ceil(alpha * static_cast<float>(possible_insertions.size())));

        Insertion selected_insertion = possible_insertions[dist_selection(m_generator)];
        Flight &selected_candidate = candidate_list[selected_insertion.candidate_i].get();

        selected_candidate.start_time = selected_insertion.start_time;
        selected_candidate.runway = selected_insertion.runway;
        selected_candidate.position = solution.runways[selected_insertion.runway].sequence.size();

        solution.runways[selected_insertion.runway].sequence.emplace_back(selected_candidate);
        solution.runways[selected_insertion.runway].penalty += selected_insertion.penalty;
        solution.runways[selected_insertion.runway].prefix_penalty.push_back(
            solution.runways[selected_insertion.runway].penalty);
        solution.objective += selected_insertion.penalty;

        candidate_list.erase(candidate_list.begin() + static_cast<long>(selected_insertion.candidate_i));
    }

    assert(solution.test_feasibility(m_instance));

    return solution;
}

Solution ASP::lowest_release_time_insertion(std::vector<Flight> &flights) {
    Solution solution(m_instance);

    // Initialization of the candidate list
    std::vector<std::reference_wrapper<Flight>> candidate_list;
    candidate_list.reserve(m_instance.get_num_flights());

    for (size_t i = 0; i < m_instance.get_num_flights(); ++i) {
        candidate_list.emplace_back(flights[i]);
    }
    std::vector<size_t> candidates_position(m_instance.get_num_flights());

    // Ordering of the candidate list by release time
    std::sort(candidate_list.begin(), candidate_list.end(), [](const auto flight_a, const auto flight_b) {
        return flight_a.get().get_release_time() > flight_b.get().get_release_time();
    });

    size_t qtd_runways = m_instance.get_num_runways();

    // Put the "runway.size()"'s lowests release time flights
    for (size_t runway_i = 0; runway_i < m_instance.get_num_runways(); ++runway_i) {
        size_t idx =  (rand() % std::min(qtd_runways, candidate_list.size())) + (candidate_list.size() - std::min(qtd_runways, candidate_list.size())); //index of the selected flight in candidate list

        solution.runways[runway_i].sequence.emplace_back(candidate_list[idx]); // Coloca ele no final daquela pista
        solution.runways[runway_i].prefix_penalty.push_back(0); // O penalty dele é zero pois é o primeiro

        Flight &candidate = candidate_list[idx].get(); // Facilitar a escrita

        // Atualiza seus valores
        candidate.position = 0;
        candidate.runway = runway_i;
        candidate.start_time = candidate.get_release_time();

        // Apaga ele da lista de candidatos
        for(size_t i = idx; i < candidate_list.size()-1; i++){
            candidate_list[i] = candidate_list[i+1];
        }
        candidate_list.pop_back();
    }

    // Insert all the flights in the solution
    size_t best_runway;
    size_t lowest_start_time;
    size_t start_time;
    while (!candidate_list.empty()) {

        // Search the best runway which the start time be the lowest possible
        best_runway = 0;
        lowest_start_time = std::numeric_limits<size_t>::max();

        size_t idx =  (rand() % std::min(qtd_runways, candidate_list.size())) + (candidate_list.size() - std::min(qtd_runways, candidate_list.size())); //index of the selected flight in candidate list
        const Flight &current_flight = candidate_list[idx].get(); // the flight who will be insert

        for (size_t runway_i = 0; runway_i < m_instance.get_num_runways(); ++runway_i) {
            const Flight &prev_flight =
                solution.runways[runway_i].sequence.back().get(); // the actual last flight in the runway

            const uint32_t earliest = prev_flight.start_time + prev_flight.get_runway_occupancy_time() +
                                      m_instance.get_separation_time(prev_flight.get_id(), current_flight.get_id());

            start_time = std::max(earliest, current_flight.get_release_time());

            if (start_time < lowest_start_time) {
                lowest_start_time = start_time;
                best_runway = runway_i;
            }
        }

        uint32_t current_flight_penalty =
            (lowest_start_time - current_flight.get_release_time()) * current_flight.get_delay_penalty();
        solution.runways[best_runway].prefix_penalty.push_back(
            solution.runways[best_runway]
                .prefix_penalty[solution.runways[best_runway].sequence.size()] // the prefix of the last flight in the
                                                                               // runwawy
            + current_flight_penalty); // O prefix penalty dele é o prefix penalty do anterior mais o penalty dele
        solution.runways[best_runway].sequence.emplace_back(candidate_list[idx]); // Coloca ele no final daquela pista

        Flight &candidate = candidate_list[idx].get(); // Facilitar a escrita

        // Atualiza seus valores
        candidate.position = solution.runways[best_runway].sequence.size() - 1;
        candidate.runway = best_runway;
        candidate.start_time = lowest_start_time;

        solution.runways[best_runway].penalty += current_flight_penalty;
        solution.objective += current_flight_penalty;

        // Apaga ele da lista de candidatos
        for(size_t i = idx; i < candidate_list.size()-1; i++){
            candidate_list[i] = candidate_list[i+1];
        }
        candidate_list.pop_back();
    }

    assert(solution.test_feasibility(m_instance));

    return solution;
}
