#include <cassert>
#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>

#include "instance.hpp"
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

    Solution solution;
    solution.schedule = {{0, 1, 4}, {2, 3, 5, 0}};
    solution.objective = 2800;

    assert(solution.test_feasibility(instance));

    return 0;
}
