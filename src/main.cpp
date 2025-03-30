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

    Solution solution;

    Runway r1;

    r1.sequence = {0, 1, 4};
    r1.penalty = 450;

    Runway r2;

    r2.sequence = {2, 3, 5};
    r2.penalty = 2350;

    solution.runways = {r1, r2};
    solution.objective = 2800;

    assert(solution.test_feasibility(instance, flights));

    Solution s2 = construction::nearest_neighbor(instance, flights);

    s2.print();

    return 0;
}
