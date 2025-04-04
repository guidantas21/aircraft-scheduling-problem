#ifndef MOVES_HPP
#define MOVES_HPP

#include <algorithm>
#include <cstddef>

#include "solution.hpp"

namespace moves {

inline void apply_inter_swap(Solution &solution, const size_t flight_i, const size_t runway_i, const size_t flight_j,
                             const size_t runway_j) {
    std::swap(solution.runways[runway_i].sequence[flight_i], solution.runways[runway_j].sequence[flight_j]);
}

inline void apply_intra_swap(Solution &solution, const size_t flight_i, const size_t flight_j, const size_t runway_i) {
    std::swap(solution.runways[runway_i].sequence[flight_i], solution.runways[runway_i].sequence[flight_j]);
}

inline void apply_inter_move();

inline void apply_intra_move();

void best_improvement_intra_swap(const Instance &instance, Solution &solution, const size_t runway_i, const std::vector<Flight> &flights);

} // namespace moves

#endif
