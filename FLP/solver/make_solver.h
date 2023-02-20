//
// Created by henri on 01/12/22.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MAKE_SOLVER_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MAKE_SOLVER_H

#include "NodeBaP.h"
#include "MostInfeasibleWithSpatialBranching.h"
#include "algorithms/dantzig-wolfe/DantzigWolfe.h"
#include "algorithms/dantzig-wolfe/BranchingManagers_OnMaster.h"
#include "MostActiveHeuristic.h"

auto get_reformulated(const Model& t_original_model, const Vector<Var, 1>& t_vec, const UserAttr& t_attr) {
    Vector<Var, 1> result;
    result.reserve(t_vec.size());

    for (const auto& var : t_vec) {
        result.emplace_back( t_original_model.get<Var>(t_attr, var) );
    }

    return result;
}

BranchAndBound make_solver(Model& t_model,
                           const UserAttr& t_flag,
                           const Vector<Var>& t_integer_branching,
                           const Vector<Var>& t_continuous_branching,
                           double t_ub) {

    const bool use_farkas_pricing = true;

    BranchAndBound result;

    auto& dantzig_wolfe = result.set_solution_strategy<DantzigWolfe>(t_model, t_flag);

    dantzig_wolfe.set(Param::DantzigWolfe::CleanUpThreshold, 500);
    dantzig_wolfe.set(Param::DantzigWolfe::SmoothingFactor, .3);
    dantzig_wolfe.set(Param::DantzigWolfe::FarkasPricing, use_farkas_pricing);
    dantzig_wolfe.set(Param::DantzigWolfe::LogFrequency, 1);

    auto& master_solver = dantzig_wolfe.set_master_solution_strategy<Solvers::Gurobi>();

    if (use_farkas_pricing) {
        master_solver.set(Param::Algorithm::InfeasibleOrUnboundedInfo, true);
    }

    for (auto& sp : dantzig_wolfe.subproblems()) {
        sp.set_exact_solution_strategy<Solvers::Gurobi>();
        sp.set_branching_manager<BranchingManagers::OnMaster>();
    }

    auto integer_branching_candidates = get_reformulated(t_model, t_integer_branching, dantzig_wolfe.reformulation().reformulated_variable());
    auto continuous_branching_candidates = get_reformulated(t_model, t_continuous_branching, dantzig_wolfe.reformulation().reformulated_variable());

    // Making continuous relaxation
    for (const auto& var : integer_branching_candidates) {
        dantzig_wolfe.reformulation().master_problem().set(Attr::Var::Type, var, Continuous);
    }

    auto& node_strategy = result.set_node_strategy<NodeStrategies::Basic<NodeBaP>>();
    node_strategy.set_active_node_manager_strategy<ActiveNodesManagers::Basic>();
    node_strategy.set_branching_strategy<MostInfeasibleWithSpatialBranching>(
            integer_branching_candidates,
            continuous_branching_candidates
            );
    node_strategy.set_node_updator_strategy<NodeUpdators::ByBoundVar>();

    result.set(Param::Algorithm::BestObjStop, t_ub);

    result.add_callback<MostActiveHeuristic>(integer_branching_candidates, continuous_branching_candidates);

    return result;

}

#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MAKE_SOLVER_H
