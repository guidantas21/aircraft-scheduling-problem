#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>
#include <random>

#include "omp.h"

namespace utils {

static thread_local std::mt19937 engine;
} // namespace utils

#endif
