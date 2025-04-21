#include "solution.hpp"
#include "instance.hpp"
#include "runway.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <unordered_set>

Solution::Solution(const Instance &instance) {
    size_t average_flights_per_runway = std::ceil(instance.get_num_flights() / instance.get_num_runways());

    runways.reserve(instance.get_num_runways());

    for (size_t runway_id = 0; runway_id < instance.get_num_runways(); ++runway_id) {
        runways.emplace_back(runway_id, average_flights_per_runway);
    }
}

uint32_t Solution::calculate_objective(const Instance &instance) const {
    uint32_t calculated_objective = 0;
    for (size_t runway_i = 0; runway_i < instance.get_num_runways(); runway_i++) {
        calculated_objective += runways[runway_i].calculate_total_penalty(instance);
    }
    return calculated_objective;
}

void Solution::update_objective(const Instance &instance) {
    uint32_t new_objective = 0;
    for (Runway &runway : runways) {
        runway.update_total_penalty(instance);
        new_objective += runway.penalty;
    }
    objective = new_objective;

    assert(test_feasibility(instance));
}

bool Solution::test_feasibility(const Instance &instance) const {
    if (runways.size() != instance.get_num_runways()) {
        return false;
    }
    std::unordered_set<size_t> flight_set;

    uint32_t real_num_flights = 0;
    uint32_t real_objective = 0;
    for (const Runway &runway : runways) {
        if (not runway.test_feasibility(instance)) {
            return false;
        }
        real_objective += runway.penalty;
        real_num_flights += runway.sequence.size();

        if (real_num_flights > instance.get_num_flights()) {
            return false;
        }

        for (const auto &flight : runway.sequence) {
            flight_set.emplace(flight.get().get_id());
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

void Solution::print(std::ofstream &fp) const {
    fp << "\nSolution\n\n";
    size_t i = 1;
    for (const Runway &runway : runways) {
        fp << ">> Runway " << i << '\n';

        runway.print(fp);
        fp << '\n';
        ++i;
    }
    fp << "Objective: " << objective << '\n';
}

void Solution::print_runway() const {
    for (const Runway &runway : runways) {
        runway.print_runway();
        std::cout << '\n';
    }
}
