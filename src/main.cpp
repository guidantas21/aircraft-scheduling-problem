#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <limits>
#include <vector>

#include "ASP.hpp"
#include "instance.hpp"
#include "solution.hpp"

constexpr size_t NUM_EXECUTIONS = 10;

struct InstanceData {
    std::string instance_file;
    int value;
    std::string type;
};

struct ExecutionsData {
    InstanceData instance_data;

    double sum_objective = 0;
    double avg_objective = 0;

    double sum_time = 0;
    double avg_time = 0;

    uint32_t best_found = std::numeric_limits<uint32_t>::max();

    double avg_gap = 0;
    double best_gap = std::numeric_limits<uint32_t>::max();

    size_t num_executions = 0;

    ExecutionsData() = default;

    double calculate_gap(uint32_t objective) const {
        return ((static_cast<double>(objective - instance_data.value)) / static_cast<double>(instance_data.value)) *
               100;
    }

    void count_execution(std::ofstream &fp, Solution &solution, double new_time) {
        sum_objective += static_cast<double>(solution.objective);
        sum_time += new_time;

        num_executions += 1;

        avg_objective = sum_objective / static_cast<double>(num_executions);
        avg_time = sum_time / static_cast<double>(num_executions);

        if (solution.objective < best_found) {
            best_found = solution.objective;
            best_gap = calculate_gap(solution.objective);
        }
        avg_gap = calculate_gap(solution.objective);
        solution.print(fp);

        fp << "Optimal (or lower bound): " << instance_data.value << '\n';
        fp << "Gap: " << calculate_gap(solution.objective) << "%\n";
        fp << "Elapsed time: " << new_time << " microseconds\n";
    }
};

void create_result_table(std::vector<InstanceData> &instances, std::vector<ExecutionsData> &construction,
                         std::vector<ExecutionsData> &local_search, std::vector<ExecutionsData> &methaheuristic

) {
    std::ofstream fp_table;

    fp_table.open("data/table.txt");

    if (!fp_table.is_open()) {
        std::cout << "Failed to open table file\n";
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < instances.size(); ++i) {
        fp_table << "\t" << instances[i].instance_file << " & " << instances[i].value << " & ";

        fp_table << construction[i].avg_objective << " & " << construction[i].best_found << " & "
                 << construction[i].avg_time << " & " << construction[i].avg_gap << " & " << construction[i].best_gap
                 << " & ";
        fp_table << local_search[i].avg_objective << " & " << local_search[i].best_found << " & "
                 << local_search[i].avg_time << " & " << local_search[i].avg_gap << " & " << local_search[i].best_gap
                 << " & ";
        fp_table << methaheuristic[i].avg_objective << " & " << methaheuristic[i].best_found << " & "
                 << methaheuristic[i].avg_time << " & " << methaheuristic[i].avg_gap << " & "
                 << methaheuristic[i].best_gap;
        fp_table << "\\\\" << '\n';
    }

    fp_table.close();
}

int main() {
    std::filesystem::path instances_dir_path("data/instances/");
    std::filesystem::path results_dir_path("data/results/");

    std::vector<InstanceData> instances = {
        {"n3m10A.txt", 7483, "opt"}, {"n3m10B.txt", 1277, "opt"}, {"n3m10C.txt", 2088, "opt"},
        {"n3m10D.txt", 322, "opt"},  {"n3m10E.txt", 3343, "opt"}, {"n3m20A.txt", 8280, "LB"},
        {"n3m20B.txt", 1820, "LB"},  {"n3m20C.txt", 855, "LB"},   {"n3m20D.txt", 4357, "opt"},
        {"n3m20E.txt", 3798, "opt"}, {"n3m40A.txt", 112, "LB"},   {"n3m40B.txt", 880, "LB"},
        {"n3m40C.txt", 1962, "LB"},  {"n3m40D.txt", 263, "LB"},   {"n3m40E.txt", 1192, "LB"},
        {"n5m50A.txt", 0, "LB"},     {"n5m50B.txt", 0, "LB"},     {"n5m50C.txt", 0, "LB"},
        {"n5m50D.txt", 0, "LB"},     {"n5m50E.txt", 0, "LB"}};

    std::vector<ExecutionsData> construction(instances.size());
    std::vector<ExecutionsData> local_search(instances.size());
    std::vector<ExecutionsData> methaheuristic(instances.size());

    size_t instance_id = 0;
    for (const InstanceData &instance_data : instances) {

        std::filesystem::path instance_file_path = instances_dir_path / instance_data.instance_file;
        std::filesystem::path result_file_path = results_dir_path / instance_data.instance_file;

        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        std::chrono::time_point<std::chrono::high_resolution_clock> end;
        std::chrono::microseconds elapsed_time;

        construction[instance_id].instance_data = instance_data;
        local_search[instance_id].instance_data = instance_data;
        methaheuristic[instance_id].instance_data = instance_data;

        Instance instance(instance_file_path);

        ASP asp(instance);

        size_t grasp_iterations = 10;
        size_t ils_iterations = 200;
        double alpha = 2 * instance.get_num_flights() / instance.get_num_runways();

        std::ofstream fp_result;

        fp_result.open(result_file_path);

        if (!fp_result.is_open()) {
            std::cout << "Failed to open instance `" << result_file_path << "`\n";
            exit(EXIT_FAILURE);
        }

        // Construction and local search

        for (size_t execution = 0; execution < NUM_EXECUTIONS; ++execution) {

            fp_result << "\n----- Construction " << execution + 1 << ": \n";
            std::cout << "\n(" << instance_data.instance_file << ") Construction " << execution + 1 << ": \n";

            start = std::chrono::high_resolution_clock::now();

            Solution solution = asp.lowest_release_time_insertion();

            end = std::chrono::high_resolution_clock::now();

            elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            construction[instance_id].count_execution(fp_result, solution, elapsed_time.count());

            fp_result << "\n----- VND " << execution + 1 << ": \n";

            std::cout << "\n(" << instance_data.instance_file << ") VND " << execution + 1 << ": \n";

            start = std::chrono::high_resolution_clock::now();

            asp.VND(solution);

            end = std::chrono::high_resolution_clock::now();

            elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            local_search[instance_id].count_execution(fp_result, solution, elapsed_time.count());
        }

        // GILS-RVND

        for (size_t execution = 0; execution < NUM_EXECUTIONS; ++execution) {
            fp_result << "\n----- GILS-RVND " << execution + 1 << ":\n";

            std::cout << "\n(" << instance_data.instance_file << ") GILS-RVND " << execution + 1 << ": \n";

            start = std::chrono::high_resolution_clock::now();

            Solution solution = asp.GILS_RVND(grasp_iterations, ils_iterations, alpha);

            end = std::chrono::high_resolution_clock::now();

            elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            methaheuristic[instance_id].count_execution(fp_result, solution, elapsed_time.count());
        }
        ++instance_id;
    }

    create_result_table(instances, construction, local_search, methaheuristic);

    return 0;
}
