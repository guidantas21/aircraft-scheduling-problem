#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>

#include "ASP.hpp"
#include "instance.hpp"
#include "solution.hpp"

int main(int argc, char *argv[]) {
    srand(time(NULL));

    argparse::ArgumentParser program("ASP");

    srand(time(nullptr));

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

    Solution s2 = asp.GILS_VND(1, 12, 0.50);

    s2.print_runway();

    std::cout << "Objective: " << s2.objective << '\n';

    // Solution s1 = asp.GILS_VND(1, 50, 0);
    // s1.print();

    return 0;
}