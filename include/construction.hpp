#ifndef CONSTRUCTION_HPP
#define CONSTRUCTION_HPP

#include "flight.hpp"
#include "instance.hpp"
#include "solution.hpp"

#include <vector>

namespace construction {

Solution nearest_neighbor(const Instance &instance, std::vector<Flight> &flights, float alpha);

Solution nearest_neighbor_2(const Instance &instance, std::vector<Flight> &flights, float alpha);

} // namespace construction

#endif
