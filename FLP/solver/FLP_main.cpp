#include <iostream>
#include "AdjustableFLP.h"
#include "solvers.h"

void solve(const std::string& t_filename, ObjectiveType t_objective_type, UncertaintySet t_uncertainty_set, double t_uncertainty_parameter) {

    std::cout << "Solving with " << t_objective_type << " objective, " << t_uncertainty_set << " uncertainty set (parameter = " << t_uncertainty_parameter << ")." << std::endl;

    auto instance = read_instance(t_filename);

    AdjustableFLP problem(instance, t_objective_type, t_uncertainty_set, t_uncertainty_parameter);

    auto& model = problem.model();

    Idol::set_optimizer<Gurobi>(model);

    model.optimize();

    const double gurobi_obj = model.get(Attr::Solution::ObjVal);
    const auto gurobi_solution = save(model, Attr::Solution::Primal);

    Idol::set_optimizer<BranchAndPriceMIP<Gurobi>>(model, problem.decomposition());
    model.set(Param::ColumnGeneration::ArtificialVarCost, model.get(Attr::Solution::ObjVal) + 1);
    model.set(Param::ColumnGeneration::LogFrequency, 1);
    model.set(Param::ColumnGeneration::BranchingOnMaster, false);
    model.set(Param::ColumnGeneration::FarkasPricing, false);
    model.set(Param::ColumnGeneration::SmoothingFactor, .3);

    model.optimize();

    const double colgen_obj = model.get(Attr::Solution::ObjVal);
    const auto colgen_solution = save(model, Attr::Solution::Primal);

    const double gap = relative_gap(gurobi_obj, colgen_obj);

    std::cout << "Gurobi: " << gurobi_obj << std::endl;
    std::cout << "ColGen: " << colgen_obj << std::endl;
    std::cout << "Gap: " << gap * 100 << " %" << std::endl;

    std::cout << save(model, Attr::Solution::Primal) << std::endl;


    if (gap > 1e-4) {
        throw Exception("Error: Gurobi and ColGen do not match.");
    }

}

int main(int t_argc, const char** t_argv) {

    Logs::set_level<BranchAndBound>(Trace);
    Logs::set_color<BranchAndBound>(Blue);

    Logs::set_level<ColumnGeneration>(Trace);
    Logs::set_color<ColumnGeneration>(Yellow);

    //const std::string filename = "/home/henri/CLionProjects/AB_AdjustableRobustOptimizationWithObjectiveUncertainty/FLP/data/instance_4_8_120__1.txt";

    const std::string folder = "/home/henri/CLionProjects/AB_AdjustableRobustOptimizationWithObjectiveUncertainty/FLP/data/";

    for (unsigned int i = 0 ; i < 5 ; ++i) {

        const std::string filename = folder + "instance_4_8_110__" + std::to_string(i) + ".txt";

        for (double budget: {1., 2., 3.}) {
            for (ObjectiveType objective_type: {Linearized /*, Convex */}) {
                for (UncertaintySet uncertainty_type: {/*Polyhedral, */ Ellipsoidal }) {
                    solve(filename, objective_type, uncertainty_type, budget);
                }
            }
        }

    }

    return 0;
}
