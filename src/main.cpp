#include <cassert>
#include <cstddef>
#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>

#include "construction.hpp"
#include "flight.hpp"
#include "instance.hpp"
#include "runway.hpp"
#include "solution.hpp"

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("ASP");

    program.add_argument("instance").help("Path to the input file").required();

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    std::filesystem::path instance_file_path = program.get<std::string>("instance");

    Instance instance(instance_file_path);

    instance.print();

    std::vector<Flight> flights;

    flights.reserve(instance.get_num_flights());

    for (size_t i = 0; i < instance.get_num_flights(); ++i) {
        flights.emplace_back(instance.get_release_time(i), instance.get_runway_occupancy_time(i),
                             instance.get_delay_penalty(i));
    }

    Solution s1(instance);

    s1.runways[0].sequence = {0, 1, 5};
    s1.runways[0].penalty = s1.runways[0].calculate_total_penalty(instance, flights);
    s1.objective += s1.runways[0].penalty;

    s1.runways[1].sequence = {2, 3, 4};
    s1.runways[1].penalty = s1.runways[1].calculate_total_penalty(instance, flights);
    s1.objective += s1.runways[1].penalty;

    assert(s1.test_feasibility(instance, flights));

    s1.print();

    Solution s2 = construction::nearest_neighbor(instance, flights);

    s2.print();

    return 0;
}
