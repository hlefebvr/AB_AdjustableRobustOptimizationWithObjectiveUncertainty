#include <iostream>
#include "AdjustableFLP.h"
#include "solvers.h"
#include "optimizers/dantzig-wolfe/DantzigWolfeDecomposition.h"
#include "optimizers/dantzig-wolfe/Optimizers_DantzigWolfeDecomposition.h"
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
            .with_time_limit(3600)
    );

    model.optimize();

    const double adjustable_model_optimal_objective = model.get(Attr::Solution::ObjVal);

    const double static_adjustable_gap = relative_gap(static_model_optimal_objective_value, adjustable_model_optimal_objective);

    const auto& branch_and_bound = model.optimizer().as<Optimizers::BranchAndBound<NodeWithActiveColumns>>();
    const auto& column_generation = branch_and_bound.relaxation().optimizer().as<Optimizers::DantzigWolfeDecomposition>();

    std::cout << "result,"
              // Instance file name
              << t_filename << ','
              // Uncertainty set type
              << t_uncertainty_set << ','
              // Uncertainty set parameter
              << t_uncertainty_parameter << ','
              // Objective type
              << t_objective_type << ','
              // Value of the static model
              << static_model_optimal_objective_value << ','
              // Value of the adjustable model
              << adjustable_model_optimal_objective << ','
              // Solution status of the adjustable model
              << (SolutionStatus) model.get(Attr::Solution::Status) << ','
              // Reason for the solution status of the adjustable model
              << (SolutionReason) model.get(Attr::Solution::Reason) << ','
              // Number of created nodes
              << branch_and_bound.n_created_nodes() << ','
              // Number of solved nodes
              << branch_and_bound.n_solved_nodes() << ','
              // Number of columns
              << column_generation.subproblems().begin()->pool().size() << ","
              // Final relative optimality gap
              << (model.get(Attr::Solution::RelGap) * 100) << ','
              // Final gap between static and adjustable model
              << (static_adjustable_gap * 100) << ','
              // Total execution time
              << model.optimizer().time().count() << ','
              // Time spent solving the master problem
              << column_generation.master().optimizer().time().cumulative_count() << ','
              // Time spent solving the pricing problem
              << column_generation.subproblems().begin()->model().optimizer().time().cumulative_count() << ','
              // Root node best bound
              << branch_and_bound.root_node_best_bound() << ','
              // Root node best obj
              << branch_and_bound.root_node_best_obj()
              << std::endl;

}

int main(int t_argc, const char** t_argv) {

    const std::string filename = t_argc == 2 ? t_argv[1] : "/home/henri/CLionProjects/AB_AdjustableRobustOptimizationWithObjectiveUncertainty/FLP/data/instance_6_12_120__2.txt";

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
