//
// Created by henri on 01/12/22.
//
#include <iostream>
#include <fstream>
#include "old_Instance.h"

int main(int t_argc, const char** t_argv) {

    using namespace old;

    if (t_argc != 2) {
        throw std::runtime_error("Expected argument 1: filename.");
    }

    const std::string filename = t_argv[1];

    Instance instance(filename);

    std::string base_filename = filename.substr(filename.find_last_of("/\\") + 1);

    std::ofstream file("/home/henri/CLionProjects/AB_AdjustableRobustOptimizationWithObjectiveUncertainty/FLP/data/" + base_filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open destination file.");
    }

    file << instance << std::endl;

    return 0;
}