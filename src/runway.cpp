#include "runway.hpp"
#include "flight.hpp"

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>

Runway::Runway(std::vector<size_t> &sequence, const size_t penalty) : sequence(sequence), penalty(penalty) {}

uint32_t Runway::calculate_total_penalty(const Instance &instance, const std::vector<Flight> &flights) const {
    size_t penalty = 0;

    if (sequence.empty()) {
        return 0;
    }
    uint32_t current_time = flights[sequence.front()].get_release_time();

    for (size_t i = 0; i < sequence.size() - 1; ++i) {
        Flight current_flight = flights[sequence[i]];

        Flight next_flight = flights[sequence[i + 1]];

        uint32_t earliest_possible = current_time + current_flight.get_runway_occupancy_time() +
                                     instance.get_separation_time(sequence[i], sequence[i + 1]);
        uint32_t release_time = next_flight.get_release_time();

        current_time = std::max(release_time, earliest_possible);

        uint32_t delay = current_time - release_time;

        penalty += next_flight.get_delay_penalty() * delay;
    }
    return penalty;
}

void Runway::update_total_penalty(const Instance &instance, const std::vector<Flight> &flights) {
    penalty = calculate_total_penalty(instance, flights);
}

bool Runway::test_sequence_feasibility(const Instance &instance) const {
    std::unordered_set<size_t> set;

    for (const size_t flight : sequence) {
        if (flight >= instance.get_num_flights()) {
            return false;
        }
        if (set.find(flight) == set.end()) {
            set.insert(flight);
        } else {
            return false;
        }
    }
    return true;
}

bool Runway::test_penalty(const Instance &instance, const std::vector<Flight> &flights) const {
    return penalty == calculate_total_penalty(instance, flights);
}

bool Runway::test_feasibility(const Instance &instance, const std::vector<Flight> &flights) const {
    return test_sequence_feasibility(instance) and test_penalty(instance, flights);
}

void Runway::print() const {
    std::cout << "Flights: ";

    for (const size_t flight : sequence) {
        std::cout << flight + 1 << ' ';
    }
    std::cout << '\n';
    std::cout << "Number of flights: " << sequence.size() << '\n';
    std::cout << "Total penalty: " << penalty << '\n';
}
