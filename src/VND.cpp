#include "ASP.hpp"

#include <cassert>
#include <csignal>
#include <cstddef>
#include <vector>

void ASP::VND(Solution &solution) { // NOLINT
    std::vector<Neighborhood> neighborhoods{Neighborhood::IntraSwap, Neighborhood::IntraMove, Neighborhood::InterSwap,
                                            Neighborhood::InterMove};

    bool improved = false;
    size_t current_neighborhood = 0;

    while (current_neighborhood < neighborhoods.size()) {

        switch (neighborhoods[current_neighborhood]) {
        case Neighborhood::IntraSwap:
            improved = best_improvement_intra_swap(solution);
            break;
        case Neighborhood::InterSwap:
            improved = best_improvement_inter_swap(solution);
            break;
        case Neighborhood::IntraMove:
            improved = best_improvement_intra_move(solution);
            break;
        case Neighborhood::InterMove:
            improved = best_improvement_inter_move(solution);
            break;
        }
        if (improved) {
            current_neighborhood = 0;
        } else {
            ++current_neighborhood;
        }
    }
    assert(solution.test_feasibility(m_instance));
}
