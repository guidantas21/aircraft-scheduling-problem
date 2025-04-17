#ifndef FLIGHT_HPP
#define FLIGHT_HPP

#include <cstddef>
#include <cstdint>

class Flight {
private:
    size_t m_id;
    uint32_t m_release_time;
    uint32_t m_runway_occupancy_time;
    uint32_t m_delay_penalty;

public:
    uint16_t start_time = 0;
    size_t runway = 0;
    size_t position = 0;

    Flight() = default;
    Flight(size_t id, uint32_t release_time, uint32_t taxing_time, uint32_t delay_penalty);

    inline size_t get_id() const { return m_id; }
    inline uint32_t get_release_time() const { return m_release_time; }
    inline uint32_t get_runway_occupancy_time() const { return m_runway_occupancy_time; }
    inline uint32_t get_delay_penalty() const { return m_delay_penalty; }
};

#endif
