#include <cassert>
#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>

#include "ASP.hpp"
#include "instance.hpp"
#include "solution.hpp"

int main(int argc, char *argv[]) {
    srand(time(NULL));

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

    /*instance.print();*/

    ASP asp(instance);

    // Solution s2 = asp.GILS_VND_2(20, 30, 0.05);
    // s2.print();

    // Solution s1 = asp.GILS_VND(1, 50, 0);
    // s1.print();

    Solution s1 = asp.lowest_release_time_insertion(asp.flights);
    s1.print();

    return 0;
}
