#include "solution.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <unordered_set>

bool Solution::test_feasibility(const Instance &instance) const {
    std::unordered_set<size_t> flight_set;

    if (schedule.size() != instance.get_num_runways()) {
        return false;
    }

    size_t real_objective = 0;
    size_t total_flights = 0;
    for (const auto &runway : schedule) {
        size_t current_time = instance.get_confirmation_time(runway[0]);
        flight_set.insert(runway[0]);

        total_flights += runway.size();

        for (size_t j = 1; j < runway.size(); ++j) {
            size_t flight = runway[j - 1];
            size_t next_flight = runway[j];

            if (flight_set.find(next_flight) == flight_set.end()) {
                flight_set.insert(next_flight);

                current_time = std::max(current_time + instance.get_taxing_time(flight) +
                                            instance.get_separation_time(flight, next_flight),
                                        instance.get_confirmation_time(next_flight));

                size_t waiting_time = current_time - instance.get_confirmation_time(next_flight);

                real_objective += instance.get_delay_penalty(next_flight) * waiting_time;
            } else {
                return false;
            }
        }
    }
    return real_objective == objective and total_flights == instance.get_num_flights();
}
