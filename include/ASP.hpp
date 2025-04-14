#ifndef ASP_HPP
#define ASP_HPP

#include <vector>

#include "flight.hpp"
#include "instance.hpp"
#include "solution.hpp"

class ASP {
private:
    Instance m_instance;

    std::vector<Flight> m_flights;

public:
    void best_improvement_intra_swap(Solution &solution, size_t runway_i);
    void best_improvement_inter_swap(Solution &solution);

    void best_improvement_intra_move(Solution &solution);

    void temp_apply_intra_move(Solution &solution, size_t flight_i, size_t flight_j, size_t runway_i);

    Solution VND(Solution &solution); // NOLINT

    Solution randomized_greedy(float alpha);

    Solution GRASP_VND(size_t max_iterations);                                        // NOLINT
    Solution GILS_VND(size_t max_iterations, size_t max_ils_iterations, float alpha); // NOLINT

    ASP(Instance &instance);
};

#endif
