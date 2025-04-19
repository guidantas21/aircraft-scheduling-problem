#include "ASP.hpp"
#include <cstdlib>
#include <random>

ASP::ASP(Instance &instance) : m_instance(instance) {
    std::random_device rd;
    m_generator = std::mt19937(rd());

    for (size_t i = 0; i < m_instance.get_num_flights(); ++i) {
        flights.emplace_back(i, m_instance.get_release_time(i), m_instance.get_runway_occupancy_time(i),
                             m_instance.get_delay_penalty(i));
    }
}
