#include "runway.hpp"

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>

#include "flight.hpp"

uint32_t Runway::calculate_total_penalty(const Instance &instance) const {
    size_t penalty = 0;
    if (sequence.empty()) {
        return 0;
    }
    uint32_t current_time = sequence.front().get().get_release_time();

    for (size_t i = 0; i < sequence.size() - 1; ++i) {
        uint32_t earliest_possible =
            current_time + sequence[i].get().get_runway_occupancy_time() +
            instance.get_separation_time(sequence[i].get().get_id(), sequence[i + 1].get().get_id());
        uint32_t release_time = sequence[i + 1].get().get_release_time();

        current_time = std::max(release_time, earliest_possible);

        uint32_t delay = current_time - release_time;

        penalty += sequence[i + 1].get().get_delay_penalty() * delay;
    }
    return penalty;
}

void Runway::update_total_penalty(const Instance &instance) { penalty = calculate_total_penalty(instance); }

bool Runway::test_sequence_feasibility(const Instance &instance) const {
    std::unordered_set<size_t> set;

    for (const auto &flight : sequence) {
        if (flight.get().get_id() >= instance.get_num_flights()) {
            return false;
        }
        if (set.find(flight.get().get_id()) == set.end()) {
            set.insert(flight.get().get_id());
        } else {
            return false;
        }
    }
    return true;
}

bool Runway::test_penalty(const Instance &instance) const { return penalty == calculate_total_penalty(instance); }

bool Runway::test_feasibility(const Instance &instance) const {
    return test_sequence_feasibility(instance) and test_penalty(instance);
}

void Runway::print() const {
    std::cout << "Flights: ";

    for (const auto &flight : sequence) {
        std::cout << flight.get().get_id() + 1 << ' ';
    }
    std::cout << '\n';
    std::cout << "Number of flights: " << sequence.size() << '\n';
    std::cout << "Total penalty: " << penalty << '\n';
}
