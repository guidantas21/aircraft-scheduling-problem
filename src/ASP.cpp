#include "ASP.hpp"

ASP::ASP(Instance &instance) : m_instance(instance) {
    for (size_t i = 0; i < m_instance.get_num_flights(); ++i) {
        m_flights.emplace_back(i, m_instance.get_release_time(i), m_instance.get_runway_occupancy_time(i),
                               m_instance.get_delay_penalty(i));
    }
}
