#include "instance.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>

Instance::Instance(std::filesystem::path &instance_file_path) : m_num_flights(0), m_num_runways(0) {
    std::ifstream file(instance_file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open instance file");
    }

    file >> m_num_flights >> m_num_runways;

    m_release_times.resize(m_num_flights);
    for (size_t i = 0; i < m_num_flights; ++i) {
        file >> m_release_times[i];
    }

    m_runway_occupancy_times.resize(m_num_flights);
    for (size_t i = 0; i < m_num_flights; ++i) {
        file >> m_runway_occupancy_times[i];
    }

    m_delay_penalties.resize(m_num_flights);
    for (size_t i = 0; i < m_num_flights; ++i) {
        file >> m_delay_penalties[i];
    }

    size_t separation_buffer = 0;

    m_separation_time_matrix.resize(m_num_flights * m_num_flights);
    for (size_t i = 0; i < m_num_flights; ++i) {
        for (size_t j = 0; j < m_num_flights; ++j) {
            file >> separation_buffer;

            m_separation_time_matrix[(i * m_num_flights) + j] = separation_buffer;
        }
    }
}

void Instance::print() const {
    std::cout << "Instance Data:\n";
    std::cout << "Number of flights: " << m_num_flights << "\n";
    std::cout << "Number of runways: " << m_num_runways << "\n\n";

    std::cout << "Release Times:\n";
    for (size_t i = 0; i < m_release_times.size(); ++i) {
        std::cout << "Flight " << i + 1 << ": " << std::setw(5) << m_release_times[i] << "\n";
    }

    std::cout << "\nRunway Occupancy Times:\n";
    for (size_t i = 0; i < m_runway_occupancy_times.size(); ++i) {
        std::cout << "Flight " << i + 1 << ": " << std::setw(5) << m_runway_occupancy_times[i] << "\n";
    }

    std::cout << "\nDelay Penalties:\n";
    for (size_t i = 0; i < m_delay_penalties.size(); ++i) {
        std::cout << "Flight " << i + 1 << ": " << std::setw(5) << m_delay_penalties[i] << "\n";
    }

    std::cout << "\nSeparation Time Matrix:\n";
    std::cout << "      ";
    for (size_t j = 0; j < m_num_flights; ++j) {
        std::cout << std::setw(5) << "F" << j + 1;
    }
    std::cout << "\n";

    for (size_t i = 0; i < m_num_flights; ++i) {
        std::cout << "F" << std::setw(3) << i + 1 << " |";
        for (size_t j = 0; j < m_num_flights; ++j) {
            std::cout << std::setw(6) << get_separation_time(i, j);
        }
        std::cout << "\n";
    }
}
