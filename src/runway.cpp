#include "runway.hpp"

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>

#include "flight.hpp"

Runway::Runway(const size_t id, const size_t estimated_size) : m_id(id) {
    sequence.reserve(estimated_size);
    prefix_penalty.reserve(estimated_size + 1);
    prefix_penalty.push_back(0);
}

uint32_t Runway::calculate_total_penalty(const Instance &instance) const {
    size_t real_penalty = 0;
    if (sequence.empty()) {
        return 0;
    }
    uint32_t start_time = sequence.front().get().get_release_time();
    for (size_t i = 1; i < sequence.size(); ++i) {
        Flight &prev_flight = sequence[i - 1].get();
        Flight &current_flight = sequence[i].get();

        uint32_t earliest_possible = start_time + prev_flight.get_runway_occupancy_time() +
                                     instance.get_separation_time(prev_flight.get_id(), current_flight.get_id());

        start_time = std::max(current_flight.get_release_time(), earliest_possible);

        uint32_t delay = start_time - current_flight.get_release_time();

        real_penalty += current_flight.get_delay_penalty() * delay;
    }
    return real_penalty;
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

void Runway::print_runway() const {
    for (const auto &flight : sequence) {
        std::cout << flight.get().get_id() + 1 << ' ';
    }
    std::cout << '\n';
}

void Runway::print() const {
    std::cout << "Flights: ";

    print_runway();

    std::cout << "Prefix Penalty: ";
    for (const auto &penalty : prefix_penalty) {
        std::cout << penalty << ' ';
    }

    std::cout << '\n';
    std::cout << "Number of flights: " << sequence.size() << '\n';
    std::cout << "Total penalty: " << penalty << '\n';
}
