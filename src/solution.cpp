#include "solution.hpp"
#include "runway.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <unordered_set>

Solution::Solution(const Instance &instance) { runways.resize(instance.get_num_runways()); }

bool Solution::test_feasibility(const Instance &instance, std::vector<Flight> &flights) const {
    if (runways.size() != instance.get_num_runways()) {
        return false;
    }
    std::unordered_set<size_t> flight_set;

    size_t real_num_flights = 0;
    size_t real_objective = 0;
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
