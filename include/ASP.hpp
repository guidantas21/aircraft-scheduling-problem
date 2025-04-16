#ifndef ASP_HPP
#define ASP_HPP

#include <random>
#include <vector>

#include "flight.hpp"
#include "instance.hpp"
#include "solution.hpp"

class ASP {
private:
    Instance m_instance;

    static thread_local std::mt19937 m_generator;

    std::vector<Flight> m_flights;

public:
    // Constructive heuristics

    Solution randomized_greedy(float alpha, std::vector<Flight> &flights);

    // Local search procedures

    void VND(Solution &solution); // NOLINT

    // Neighborhoods

    bool best_improvement_intra_swap(Solution &solution);
    bool best_improvement_inter_swap(Solution &solution);
    void temp_apply_intra_move(Solution &solution, size_t flight_i, size_t flight_j, size_t runway_i);
    bool best_improvement_intra_move(Solution &solution);
    bool best_improvement_inter_move(Solution &solution);

    // Methaheuristics

    Solution GRASP_VND(size_t max_iterations);                                                 // NOLINT
    Solution parallel_GILS_VND(size_t max_iterations, size_t max_ils_iterations, float alpha); // NOLINT
    Solution GILS_VND(size_t max_iterations, size_t max_ils_iterations, float alpha);          // NOLINT

    ASP(Instance &instance);
};

#endif
