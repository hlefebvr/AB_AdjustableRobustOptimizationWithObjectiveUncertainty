#include <iostream>
#include "AdjustableFLP.h"
#include "solvers.h"
#include "optimizers/dantzig-wolfe/DantzigWolfeDecomposition.h"
#include "optimizers/branch-and-bound/branching-rules/factories/MostInfeasible.h"
#include "optimizers/branch-and-bound/node-selection-rules/factories/WorstBound.h"
#include "../solver/NodeWithActiveColumns.h"
#include "../solver/MostInfeasibleWithSpatialBranching.h"
#include "../solver/MostActiveHeuristic.h"

void solve(const std::string& t_filename, ObjectiveType t_objective_type, UncertaintySet t_uncertainty_set, double t_uncertainty_parameter) {

    std::cout << "Solving " << t_filename << " with " << t_objective_type << " objective, " << t_uncertainty_set << " uncertainty set (parameter = " << t_uncertainty_parameter << ")." << std::endl;

    auto instance = read_instance(t_filename);

    AdjustableFLP problem(instance, t_objective_type, t_uncertainty_set, t_uncertainty_parameter);

    auto& model = problem.model();
    const auto& x = problem.x();
    const auto& q = problem.q();

    model.use(Gurobi());
    model.optimize();

    const double static_model_optimal_objective_value = model.get(Attr::Solution::ObjVal);

    model.use(
        BranchAndBound<NodeWithActiveColumns>()
            .with_node_solver(
                DantzigWolfeDecomposition(problem.decomposition())
                    .with_master_solver(Mosek::ContinuousRelaxation())
                    .with_pricing_solver(Gurobi())
                    .with_farkas_pricing(true)
                    .with_dual_price_smoothing_stabilization(.3)
                    .with_branching_on_master(false)
                    .with_log_level(Mute, Yellow)
                    //.with_log_frequency(1)
            )
            .with_branching_rule(MostInfeasibleWithSpatialBranching(x.begin(), x.end(), q.begin(), q.end()))
            .with_node_selection_rule(WorstBound())
            .with_best_bound_stop(static_model_optimal_objective_value)
            .with_subtree_depth(0)
            .with_log_level(Info, Default)
            //dd -.with_log_frequency(1)
            .with_callback(MostActiveHeuristic(x.begin(), x.end(), q.begin(), q.end()))
    );

    model.optimize();

    const double adjustable_model_optimal_objective = model.get(Attr::Solution::ObjVal);

    const double gap = relative_gap(static_model_optimal_objective_value, adjustable_model_optimal_objective);

    std::cout << "Gurobi: " << static_model_optimal_objective_value << std::endl;
    std::cout << "ColGen: " << adjustable_model_optimal_objective << std::endl;
    std::cout << "Gap: " << gap * 100 << " %" << std::endl;

    std::cout << "result,"
              << t_filename << ','
              << t_uncertainty_set << ','
              << t_uncertainty_parameter << ','
              << t_objective_type << ','
              << static_model_optimal_objective_value << ','
              << adjustable_model_optimal_objective << ','
              << (model.get(Attr::Solution::RelGap) * 100) << ','
              << (gap * 100) << ','
              << model.optimizer().time().count()
              << std::endl;

}

int main(int t_argc, const char** t_argv) {

    const std::string filename = "/home/henri/CLionProjects/AB_AdjustableRobustOptimizationWithObjectiveUncertainty/FLP/data/instance_6_12_120__2.txt";

    for (const auto objective_type : {Convex, Linearized }) {
        for (const auto unceratinty_set_type : {Ellipsoidal, Polyhedral }) {
            for (double Gamma : { 1., 2., 3., 4. }) {
                try {
                    solve(filename, objective_type, unceratinty_set_type, Gamma);
                } catch (...) {
                    std::cout << "Failed instance " << filename << " with " << objective_type << " objective "
                              << " and " << unceratinty_set_type << " uncertainty type with parameter "
                              << Gamma << "." << std::endl;
                }
            }
        }
    }

    return 0;
}
