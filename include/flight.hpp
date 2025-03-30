#ifndef FLIGHT_HPP
#define FLIGHT_HPP

#include "instance.hpp"

#include <cstdint>

class Flight {
private:
    uint32_t m_confirmation_time;
    uint32_t m_taxing_time;
    uint32_t m_delay_penalty;

public:
    size_t runway = 0;
    size_t position = 0;

    Flight(uint32_t confirmation_time, uint32_t taxing_time, uint32_t delay_penalty);

    inline uint32_t get_confirmation_time() const { return m_confirmation_time; }
    inline uint32_t get_taxing_time() const { return m_taxing_time; }
    inline uint32_t get_delay_penalty() const { return m_delay_penalty; }
};

#endif
