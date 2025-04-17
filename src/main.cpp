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

    // Solution s1 = asp.GILS_VND(1, 1, 0.05);
    Solution s1 = asp.randomized_greedy(0, asp.flights);
    s1.print();
    Solution s2 = asp.GILS_VND(100, 100, 0.05);
    s2.print();

    /*std::cout << "\n>> Constructive only:\n";*/

    /*Solution s1 = asp.randomized_greedy(0);*/
    /**/
    /*s1.print();*/
    /**/
    /*std::cout << "\n>> VND:\n";*/
    /**/
    /*Solution vnd_s1 = asp.VND(s1);*/
    /**/
    /*vnd_s1.print();*/
    /**/
    /*std::cout << "\n>> GRASP-VND:\n";*/
    /**/
    /*Solution grasp_vnd = asp.GRASP_VND(10);*/
    /**/
    /*grasp_vnd.print();*/

    return 0;
}
