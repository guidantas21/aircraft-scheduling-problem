#include "construction.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>

struct Insertion {
    size_t flight;
    size_t runway;
    size_t penalty;
    uint32_t start_time;

    Insertion(size_t flight, size_t runway, size_t penalty, uint32_t start_time)
        : flight(flight), runway(runway), penalty(penalty), start_time(start_time) {}
};

Solution construction::nearest_neighbor(const Instance &instance, std::vector<Flight> &flights, const float alpha) {
    Solution solution(instance);

    std::vector<size_t> candidate_list(instance.get_num_flights());
    std::vector<size_t> candidates_position(instance.get_num_flights());

    for (size_t i = 0; i < instance.get_num_flights(); ++i) {
        candidate_list[i] = i;
    }
    std::sort(candidate_list.begin(), candidate_list.end(), [&instance](const size_t a, const size_t b) {
        return instance.get_release_time(a) > instance.get_release_time(b);
    });

    for (size_t runway_i = 0; runway_i < instance.get_num_runways(); ++runway_i) {
        solution.runways[runway_i].sequence.push_back(candidate_list.back());

        flights[candidate_list.back()].runway = runway_i;
        flights[candidate_list.back()].start_time = flights[candidate_list.back()].get_release_time();

        candidate_list.pop_back();
    }

    while (not candidate_list.empty()) {
        std::vector<Insertion> possible_insertions;

        possible_insertions.reserve(instance.get_num_runways() * candidate_list.size());

        for (int candidate_i = (int)candidate_list.size() - 1; candidate_i >= 0; --candidate_i) {
            size_t candidate = candidate_list[candidate_i];
            candidates_position[candidate] = candidate_i;

            for (size_t runway_i = 0; runway_i < solution.runways.size(); ++runway_i) {
                size_t last_flight = solution.runways[runway_i].sequence.back();

                uint32_t earliest = flights[last_flight].start_time + flights[last_flight].get_runway_occupancy_time() +
                                    instance.get_separation_time(last_flight, candidate);

                uint32_t start_time = std::max(earliest, flights[candidate].get_release_time());

                uint32_t delay = start_time - flights[candidate].get_release_time();

                size_t insertion_penalty = flights[candidate].get_delay_penalty() * delay;

                possible_insertions.emplace_back(candidate, runway_i, insertion_penalty, start_time);
            }
        }
        std::sort(possible_insertions.begin(), possible_insertions.end(),
                  [&instance](const Insertion &a, const Insertion &b) {
                      if (a.penalty != b.penalty) {
                          return a.penalty < b.penalty;
                      }
                      return instance.get_release_time(a.flight) < instance.get_release_time(b.flight);
                  });

        std::uniform_int_distribution<size_t> dist_selection(
            0, std::ceil(alpha * static_cast<float>(possible_insertions.size())));

        Insertion selected_insertion = possible_insertions[dist_selection(utils::engine)];

        solution.runways[selected_insertion.runway].sequence.push_back(selected_insertion.flight);
        flights[selected_insertion.flight].start_time = selected_insertion.start_time;
        solution.runways[selected_insertion.runway].penalty += selected_insertion.penalty;
        solution.objective += selected_insertion.penalty;

        std::swap(candidate_list[candidates_position[selected_insertion.flight]], candidate_list.back());
        candidate_list.pop_back();
    }
    assert(solution.test_feasibility(instance, flights));
    return solution;
}

Solution construction::nearest_neighbor_2(const Instance &instance, std::vector<Flight> &flights, const float alpha) {
    Solution solution(instance);

    std::vector<size_t> candidate_list(instance.get_num_flights());
    std::vector<size_t> candidates_position(instance.get_num_flights());

    for (size_t i = 0; i < instance.get_num_flights(); ++i) {
        candidate_list[i] = i;
    }
    std::sort(candidate_list.begin(), candidate_list.end(), [&instance](const size_t a, const size_t b) {
        return instance.get_release_time(a) > instance.get_release_time(b);
    });

    for (size_t runway_i = 0; runway_i < instance.get_num_runways(); ++runway_i) {
        solution.runways[runway_i].sequence.push_back(candidate_list.back());

        flights[candidate_list.back()].runway = runway_i;
        flights[candidate_list.back()].start_time = flights[candidate_list.back()].get_release_time();

        candidate_list.pop_back();
    }

    while (not candidate_list.empty()) {
        std::vector<Insertion> possible_insertions;

        possible_insertions.reserve(candidate_list.size());

        size_t best_runway = 0;
        for (size_t runway_i = 1; runway_i < solution.runways.size(); ++runway_i) {
            if (solution.runways[runway_i].penalty < solution.runways[best_runway].penalty) {
                best_runway = runway_i;
            }
        }

        for (int candidate_i = (int)candidate_list.size() - 1; candidate_i >= 0; --candidate_i) {
            size_t candidate = candidate_list[candidate_i];

            candidates_position[candidate] = candidate_i;

            size_t last_flight = solution.runways[best_runway].sequence.back();

            uint32_t earliest = flights[last_flight].start_time + flights[last_flight].get_runway_occupancy_time() +
                                instance.get_separation_time(last_flight, candidate);

            uint32_t start_time = std::max(earliest, flights[candidate].get_release_time());

            uint32_t delay = start_time - flights[candidate].get_release_time();

            size_t insertion_penalty = flights[candidate].get_delay_penalty() * delay;

            possible_insertions.emplace_back(candidate, best_runway, insertion_penalty, start_time);

            std::sort(possible_insertions.begin(), possible_insertions.end(),
                      [&instance](const Insertion &a, const Insertion &b) {
                          if (a.penalty != b.penalty) {
                              return a.penalty < b.penalty;
                          }
                          return instance.get_release_time(a.flight) < instance.get_release_time(b.flight);
                      });
        }
        std::uniform_int_distribution<size_t> dist_selection(
            0, std::ceil(alpha * static_cast<float>(possible_insertions.size())));

        Insertion selected_insertion = possible_insertions[dist_selection(utils::engine)];

        solution.runways[selected_insertion.runway].sequence.push_back(selected_insertion.flight);
        flights[selected_insertion.flight].start_time = selected_insertion.start_time;
        solution.runways[selected_insertion.runway].penalty += selected_insertion.penalty;
        solution.objective += selected_insertion.penalty;

        std::swap(candidate_list[candidates_position[selected_insertion.flight]], candidate_list.back());
        candidate_list.pop_back();
    }
    assert(solution.test_feasibility(instance, flights));
    return solution;
}
