#include "solution.hpp"
#include "instance.hpp"
#include "runway.hpp"

#include <cmath>
#include <cstddef>
#include <iostream>
#include <unordered_set>

Solution::Solution(const Instance &instance) {
    runways.resize(instance.get_num_runways());

    size_t average_flights_per_runway = std::ceil(instance.get_num_flights() / instance.get_num_runways());

    for (Runway &runway : runways) {
        runway.sequence.reserve(average_flights_per_runway);
    }
}

uint32_t Solution::calculate_objective(const Instance &instance, const std::vector<Flight> &flights) const {
    uint32_t calculated_objective = 0;
    for (size_t runway_i = 0; runway_i < instance.get_num_runways(); runway_i++) {
        calculated_objective += runways[runway_i].calculate_total_penalty(instance, flights);
    }
    return calculated_objective;
}

void Solution::update_objective(const Instance &instance, const std::vector<Flight> &flights) {
    uint32_t new_objective = 0;
    for (Runway &runway : runways) {
        runway.update_total_penalty(instance, flights);
        new_objective += runway.penalty;
    }
    objective = new_objective;
}

bool Solution::test_feasibility(const Instance &instance, const std::vector<Flight> &flights) const {
    if (runways.size() != instance.get_num_runways()) {
        return false;
    }
    std::unordered_set<size_t> flight_set;

    uint32_t real_num_flights = 0;
    uint32_t real_objective = 0;
    for (const Runway &runway : runways) {
        if (not runway.test_feasibility(instance, flights)) {
            return false;
        }
        real_objective += runway.penalty;
        real_num_flights += runway.sequence.size();

        if (real_num_flights > instance.get_num_flights()) {
            return false;
        }

        for (const size_t flight : runway.sequence) {
            flight_set.insert(flight);
        }
    }
    return objective == real_objective and flight_set.size() == instance.get_num_flights() and
           real_num_flights == instance.get_num_flights();
}

void Solution::print() const {
    std::cout << "\nSolution\n\n";
    size_t i = 1;
    for (const Runway &runway : runways) {
        std::cout << ">> Runway " << i << '\n';

        runway.print();
        std::cout << '\n';
        ++i;
    }
    std::cout << "Objective: " << objective << '\n';
}
