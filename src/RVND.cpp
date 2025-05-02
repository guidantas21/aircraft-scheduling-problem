#include "ASP.hpp"
#include <cassert>

void ASP::RVND(Solution &solution) { // NOLINT
    std::vector<Neighborhood> neighborhoods{Neighborhood::IntraSwap, Neighborhood::InterSwap, Neighborhood::IntraMove,
                                            Neighborhood::InterMove};

    bool improved = false;
    size_t current_neighborhood = 0;

    while (not neighborhoods.empty()) {
        current_neighborhood = rand() % neighborhoods.size();
        switch (neighborhoods[current_neighborhood]) {
        case Neighborhood::IntraSwap:
            improved = best_improvement_intra_swap(solution);
            // improved = first_improvement_intra_swap(solution);
            break;
        case Neighborhood::InterSwap:
            improved = best_improvement_inter_swap(solution);
            // improved = first_improvement_inter_swap(solution);
            break;
        case Neighborhood::IntraMove:
            improved = best_improvement_intra_move(solution);
            // improved = first_improvement_intra_move(solution);
            break;
        case Neighborhood::InterMove:
            improved = best_improvement_inter_move(solution);
            // improved = first_improvement_inter_move(solution);
            break;
        }
        if (improved) {
            neighborhoods = {Neighborhood::IntraSwap, Neighborhood::InterSwap, Neighborhood::IntraMove,
                             Neighborhood::InterMove};

        } else {
            neighborhoods.erase(neighborhoods.begin() + static_cast<long>(current_neighborhood));
        }
    }
    assert(solution.test_feasibility(m_instance));
}
