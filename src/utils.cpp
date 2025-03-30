#include "utils.hpp"

#include <chrono>
#include <functional>
#include <random>
#include <thread>

thread_local std::mt19937
    engine(std::hash<std::thread::id>{}(std::this_thread::get_id()) ^ // NOLINT
           static_cast<unsigned long>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
