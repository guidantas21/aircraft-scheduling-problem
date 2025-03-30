#include "flight.hpp"

#include <cstdint>

Flight::Flight(uint32_t confirmation_time, uint32_t taxing_time, uint32_t delay_penalty)
    : m_confirmation_time(confirmation_time), m_taxing_time(taxing_time), m_delay_penalty(delay_penalty) {}
