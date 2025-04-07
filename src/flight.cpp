#include "flight.hpp"

#include <cstdint>

Flight::Flight(size_t id, uint32_t release_time, uint32_t taxing_time, uint32_t delay_penalty)
    : m_id(id), m_release_time(release_time), m_runway_occupancy_time(taxing_time), m_delay_penalty(delay_penalty) {}
