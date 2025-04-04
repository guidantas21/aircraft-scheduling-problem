#include <cassert>
#include <cstddef>
#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>

#include "construction.hpp"
#include "flight.hpp"
#include "instance.hpp"
#include "moves.hpp"
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

    Solution s1 = construction::nearest_neighbor(instance, flights, 0);

    s1.print();

    Solution s2 = construction::nearest_neighbor_2(instance, flights, 0);

    s2.print();

    assert(s2.test_feasibility(instance, flights));

    s2.print();

    Solution s3 = construction::nearest_neighbor(instance, flights, 0);
    
    for (int i = 0; i < s3.runways.size(); i++) {
        moves::best_improvement_intra_swap(instance, s3, i, flights);
    }

    s3.print();

    assert(s3.test_feasibility(instance, flights));

    return 0;
}
