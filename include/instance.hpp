#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include <cstddef>
#include <filesystem>
#include <vector>

class Instance {
private:
    size_t m_num_flights;
    size_t m_num_runways;
    std::vector<float> m_confirmation_time;
    std::vector<float> m_taxing_time;
    std::vector<float> m_delay_penalty;
    std::vector<std::vector<float>> m_separation_time_matrix;

public:
    Instance(std::filesystem::path &instance_file_path);

    void print() const;
};

#endif
