#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

class Instance {
private:
    size_t m_num_flights;
    size_t m_num_runways;
    std::vector<uint32_t> m_confirmation_times;
    std::vector<uint32_t> m_taxing_times;
    std::vector<uint32_t> m_delay_penalties;
    std::vector<uint32_t> m_separation_time_matrix;

public:
    Instance(std::filesystem::path &instance_file_path);

    inline size_t get_num_flights() const { return m_num_flights; }
    inline size_t get_num_runways() const { return m_num_runways; }
    inline uint32_t get_confirmation_time(size_t flight) const { return m_confirmation_times[flight]; }
    inline uint32_t get_taxing_time(size_t flight) const { return m_taxing_times[flight]; }
    inline uint32_t get_delay_penalty(size_t flight) const { return m_delay_penalties[flight]; }
    inline uint32_t get_separation_time(size_t flight_a, size_t flight_b) const {
        return m_separation_time_matrix[(flight_a * m_num_flights) + flight_b];
    }

    void print() const;
};

#endif
