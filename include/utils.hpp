#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>
#include <functional>
#include <random>
#include <thread>

namespace utils {

thread_local std::mt19937
    engine(std::hash<std::thread::id>{}(std::this_thread::get_id()) ^ // NOLINT
           static_cast<unsigned long>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

} // namespace utils

#endif
