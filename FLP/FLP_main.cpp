#include <iostream>
#include <filesystem>
#include "AdjustableFLP.h"
#include "solvers.h"
#include "../solver/Solver.h"

void solve(const std::string& t_filename, ObjectiveType t_objective_type, UncertaintySet t_uncertainty_set, double t_uncertainty_parameter) {

    std::cout << "Solving " << t_filename << " with " << t_objective_type << " objective, " << t_uncertainty_set << " uncertainty set (parameter = " << t_uncertainty_parameter << ")." << std::endl;

    auto instance = read_instance(t_filename);

    AdjustableFLP problem(instance, t_objective_type, t_uncertainty_set, t_uncertainty_parameter);

    auto& model = problem.model();

    Idol::set_optimizer<Gurobi>(model);

    model.optimize();

    const double gurobi_obj = model.get(Attr::Solution::ObjVal);
    const auto gurobi_solution = save(model, Attr::Solution::Primal);

    Idol::set_optimizer<Solver<Mosek, Gurobi>>(model, problem.decomposition());
    model.set(Param::Algorithm::BestBoundStop, gurobi_obj);
    model.set(Param::ColumnGeneration::ArtificialVarCost, gurobi_obj + 1);
    model.set(Param::ColumnGeneration::BranchingOnMaster, false);
    model.set(Param::ColumnGeneration::FarkasPricing, true);
    model.set(Param::ColumnGeneration::SmoothingFactor, .3);
    model.set(Param::ColumnGeneration::LogFrequency, 1);

    model.optimize();

    const double colgen_obj = model.get(Attr::Solution::ObjVal);
    const auto colgen_solution = save(model, Attr::Solution::Primal);

    const double gap = relative_gap(gurobi_obj, colgen_obj);

    std::cout << "Gurobi: " << gurobi_obj << std::endl;
    std::cout << "ColGen: " << colgen_obj << std::endl;
    std::cout << "Gap: " << gap * 100 << " %" << std::endl;

    std::cout << "result,"
              << t_filename << ','
              << t_uncertainty_set << ','
              << t_uncertainty_parameter << ','
              << t_objective_type << ','
              << gurobi_obj << ','
              << colgen_obj << ','
              << (model.get(Attr::Solution::RelGap) * 100) << ','
              << (gap * 100) << ','
              << model.time().count()
              << std::endl;

    std::cout << colgen_solution << std::endl;

    if (gap > 1e-4) {
        //throw Exception("Error: Gurobi and ColGen do not match.");
    }

}

int main(int t_argc, const char** t_argv) {

    Logs::set_level<BranchAndBound>(Trace);
    Logs::set_color<BranchAndBound>(Blue);

    Logs::set_level<ColumnGeneration>(Trace);
    Logs::set_color<ColumnGeneration>(Yellow);

    const std::string filename = "/home/henri/CLionProjects/AB_AdjustableRobustOptimizationWithObjectiveUncertainty/FLP/data/instance_4_8_120__1.txt";

    solve(filename, Convex, Polyhedral, 1);

    return 0;
}
