#include "construction.hpp"
#include <algorithm>
#include <cstddef>

Solution construction::cheapest_insertion(const Instance &instance) {
    Solution solution;

    std::vector<size_t> candidate_list(instance.get_num_flights());

    for (size_t i = 0; i < instance.get_num_flights(); ++i) {
        candidate_list[i] = i;
    }
    std::sort(candidate_list.begin(), candidate_list.end(), [instance](const size_t a, const size_t b) {
        return instance.get_confirmation_time(a) < instance.get_confirmation_time(b);
    });

    while (not candidate_list.empty()) {
    }
}
