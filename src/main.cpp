#include <cassert>
#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>

#include "ASP.hpp"
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

    ASP asp(instance);

    std::cout << "\n>> Constructive only:\n";

    Solution s1 = asp.randomized_greedy(0);

    s1.print();

    std::cout << "\n>> VND:\n";

    Solution vnd_s1 = asp.VND(s1);

    vnd_s1.print();

    std::cout << "\n>> GRASP-VND:\n";

    Solution grasp_vnd = asp.GRASP_VND(10);

    grasp_vnd.print();

    std::cout << "\n GILS-VND\n";

    Solution gils_vnd = asp.GILS_VND(10, 10, 0.025);

    gils_vnd.print();

    return 0;
}
