#include "ASP.hpp"
#include <cassert>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <vector>

enum class Neighborhood : uint8_t { IntraSwap, InterSwap, IntraMove };

Solution ASP::VND(Solution &solution) { // NOLINT
    std::vector<Neighborhood> neighborhoods{Neighborhood::IntraSwap, Neighborhood::InterSwap, Neighborhood::IntraMove};

    size_t current_neighborhood = 0;

    Solution best_solution = solution;

    while (current_neighborhood < neighborhoods.size()) {
        switch (neighborhoods[current_neighborhood]) {
        case Neighborhood::IntraSwap:
            for (size_t runway_i = 0; runway_i < solution.runways.size(); ++runway_i) {
                best_improvement_intra_swap(solution, runway_i);
            }
            break;
        case Neighborhood::InterSwap:
            best_improvement_inter_swap(solution);
            break;
        case Neighborhood::IntraMove:
            best_improvement_intra_move(solution);
            break;
        }
        if (solution.objective < best_solution.objective) {
            current_neighborhood = 0;
            best_solution = solution;
        } else {
            ++current_neighborhood;
        }
    }
    assert(best_solution.test_feasibility(m_instance));

    return best_solution;
}
