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

    m_confirmation_time.resize(m_num_flights);
    for (size_t i = 0; i < m_num_flights; ++i) {
        file >> m_confirmation_time[i];
    }

    m_taxing_time.resize(m_num_flights);
    for (size_t i = 0; i < m_num_flights; ++i) {
        file >> m_taxing_time[i];
    }

    m_delay_penalty.resize(m_num_flights);
    for (size_t i = 0; i < m_num_flights; ++i) {
        file >> m_delay_penalty[i];
    }

    m_separation_time_matrix.resize(m_num_flights);
    for (size_t i = 0; i < m_num_flights; ++i) {
        m_separation_time_matrix[i].resize(m_num_flights);
        for (size_t j = 0; j < m_num_flights; ++j) {
            file >> m_separation_time_matrix[i][j];
        }
    }
}

void Instance::print() const {
    std::cout << "Instance Data:\n";
    std::cout << "Number of flights: " << m_num_flights << "\n";
    std::cout << "Number of runways: " << m_num_runways << "\n\n";

    std::cout << "Confirmation Times:\n";
    for (size_t i = 0; i < m_confirmation_time.size(); ++i) {
        std::cout << "Flight " << i << ": " << std::setw(5) << m_confirmation_time[i] << "\n";
    }

    std::cout << "\nTaxing Times:\n";
    for (size_t i = 0; i < m_taxing_time.size(); ++i) {
        std::cout << "Flight " << i << ": " << std::setw(5) << m_taxing_time[i] << "\n";
    }

    std::cout << "\nDelay Penalties:\n";
    for (size_t i = 0; i < m_delay_penalty.size(); ++i) {
        std::cout << "Flight " << i << ": " << std::setw(5) << m_delay_penalty[i] << "\n";
    }

    std::cout << "\nSeparation Time Matrix:\n";
    std::cout << "       ";
    for (size_t j = 0; j < m_num_flights; ++j) {
        std::cout << std::setw(6) << "F" << j;
    }
    std::cout << "\n";

    for (size_t i = 0; i < m_separation_time_matrix.size(); ++i) {
        std::cout << "F" << std::setw(3) << i << " |";
        for (float time : m_separation_time_matrix[i]) {
            std::cout << std::setw(6) << time;
        }
        std::cout << "\n";
    }
}
