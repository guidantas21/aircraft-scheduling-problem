#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>

#include "ASP.hpp"
#include "instance.hpp"
#include "solution.hpp"

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("ASP");

    srand(time(nullptr));

    program.add_argument("instance").help("Path to the input file").required();
    program.add_argument("--grasp")
        .help("Number of GRASP iterations")
        .default_value("1")   // or no default if you want it required
        .scan<'i', size_t>(); // 'i' means integer

    program.add_argument("--ils").help("Number of ILS iterations").default_value("10").scan<'i', size_t>();

    program.add_argument("--alpha")
        .help("Alpha value for GRASP (0.0 to 1.0)")
        .default_value("0.01")
        .scan<'g', double>(); // 'g' means double (float)

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    std::filesystem::path instance_file_path = program.get<std::string>("instance");

    auto grasp_iterations = program.get<size_t>("--grasp");

    auto ils_iterations = program.get<size_t>("--ils");

    auto alpha = program.get<double>("--alpha");

    Instance instance(instance_file_path);

    /*instance.print();*/

    ASP asp(instance);

    Solution s2 = asp.GILS_RVND(grasp_iterations, ils_iterations, alpha);

    s2.print_runway();

    std::cout << "Objective: " << s2.objective << '\n';

    // Solution s1 = asp.GILS_VND(1, 50, 0);
    // s1.print();

    return 0;
}
