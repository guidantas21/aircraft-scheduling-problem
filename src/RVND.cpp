#include "ASP.hpp"
#include <cassert>
#include <cstddef>
#include <random>

void ASP::RVND(Solution &solution) { // NOLINT
    std::vector<Neighborhood> neighborhoods{Neighborhood::IntraSwap, Neighborhood::IntraMove, Neighborhood::InterSwap,
                                            Neighborhood::InterMove};

    bool improved = false;
    size_t current_neighborhood = 0;

    while (not neighborhoods.empty()) {

        std::uniform_int_distribution<size_t> dist_neighborhood(0, neighborhoods.size() - 1);
        current_neighborhood = dist_neighborhood(m_generator);

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
            neighborhoods = {Neighborhood::IntraSwap, Neighborhood::IntraMove, Neighborhood::InterSwap,
                             Neighborhood::InterMove};
        } else {
            neighborhoods.erase(neighborhoods.begin() + static_cast<long>(current_neighborhood));
        }
    }
    assert(solution.test_feasibility(m_instance));
}
