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

    std::mt19937 m_generator;

    Flight m_dummy_flight;

public:
    enum class Neighborhood : uint8_t { IntraSwap, InterSwap, IntraMove, InterMove };
    std::vector<Flight> flights;

    // Constructive heuristics

    Solution randomized_greedy(double alpha, std::vector<Flight> &flights);
    Solution lowest_release_time_insertion(std::vector<Flight> &flights);
    Solution rand_lowest_release_time_insertion(std::vector<Flight> &flights);

    // Local search procedures

    void VND(Solution &solution);  // NOLINT
    void RVND(Solution &solution); // NOLINT

    // Neighborhoods

    bool best_improvement_intra_swap(Solution &solution);
    bool best_improvement_inter_swap(Solution &solution);
    bool best_improvement_intra_move(Solution &solution);
    bool best_improvement_inter_move(Solution &solution);

    // Methaheuristics

    Solution GRASP_VND(size_t max_iterations);                                                 // NOLINT
    Solution parallel_GILS_VND(size_t max_iterations, size_t max_ils_iterations, float alpha); // NOLINT
    Solution GILS_VND(size_t max_iterations, size_t max_ils_iterations, double alpha);         // NOLINT
    Solution GILS_VND_2(size_t max_iterations, size_t max_ils_iterations, double alpha);       // NOLINT
    Solution GILS_RVND(size_t max_iterations, size_t max_ils_iterations, double alpha);        // NOLINT

    // Perturbations

    void random_inter_block_swap(Solution &solution);

    ASP(Instance &instance);
};

#endif
