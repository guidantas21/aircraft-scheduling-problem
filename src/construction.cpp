#include "ASP.hpp"

#include "flight.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <random>

Solution ASP::lowest_release_time_insertion() {
    Solution solution(m_instance);

    // Initialization of the candidate list
    std::vector<std::reference_wrapper<Flight>> candidate_list;
    candidate_list.reserve(m_instance.get_num_flights());

    for (size_t i = 0; i < m_instance.get_num_flights(); ++i) {
        candidate_list.emplace_back(flights[i]);
    }
    std::vector<size_t> candidates_position(m_instance.get_num_flights());

    // Ordering
    std::uniform_int_distribution<int> dist(0, 10);

    int i = dist(m_generator);

    if (i < 6) {
        std::sort(candidate_list.begin(), candidate_list.end(), [](const auto flight_a, const auto flight_b) {
            return flight_a.get().get_release_time() + flight_a.get().get_runway_occupancy_time() >
                   flight_b.get().get_release_time() + flight_b.get().get_runway_occupancy_time();
        });
    } else {
        std::sort(candidate_list.begin(), candidate_list.end(), [](const auto flight_a, const auto flight_b) {
            return flight_a.get().get_release_time() > flight_b.get().get_release_time();
        });
    }

    for (size_t i = candidate_list.size() - 1; i >= candidate_list.size() - m_instance.get_num_runways(); i--) {
        int prob_swap = std::rand() % 10;
        if (prob_swap < 3) {
            std::swap(candidate_list[i], candidate_list[i - m_instance.get_num_runways()]);
        }
    }

    // Put the "runway.size()"'s lowests release time flights
    for (size_t runway_i = 0; runway_i < m_instance.get_num_runways(); ++runway_i) {
        solution.runways[runway_i].sequence.emplace_back(candidate_list.back()); // Coloca ele no final daquela pista
        solution.runways[runway_i].prefix_penalty.push_back(0); // O penalty dele é zero pois é o primeiro

        Flight &candidate = candidate_list.back().get(); // Facilitar a escrita

        candidate.position = 0;
        candidate.runway = runway_i;
        candidate.start_time = candidate.get_release_time();

        candidate_list.pop_back();
    }

    // Insert all the flights in the solution
    size_t best_runway = 0;
    size_t lowest_start_time = 0;
    size_t start_time = 0;
    while (!candidate_list.empty()) {

        // Search the best runway which the start time be the lowest possible
        best_runway = 0;
        lowest_start_time = std::numeric_limits<size_t>::max();

        const Flight &current_flight = candidate_list.back().get(); // the flight who will be insert

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
                                                                               // runway
            + current_flight_penalty);
        solution.runways[best_runway].sequence.emplace_back(candidate_list.back());

        Flight &candidate = candidate_list.back().get();

        candidate.position = solution.runways[best_runway].sequence.size() - 1;
        candidate.runway = best_runway;
        candidate.start_time = lowest_start_time;

        solution.runways[best_runway].penalty += current_flight_penalty;
        solution.objective += current_flight_penalty;

        candidate_list.pop_back();
    }

    assert(solution.test_feasibility(m_instance));

    return solution;
}
