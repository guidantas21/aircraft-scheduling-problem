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
    void best_improvement_inter_move(Solution &solution);

    Solution VND(Solution &solution); // NOLINT

    Solution randomized_greedy(float alpha);

    ASP(Instance &instance);
};

#endif
