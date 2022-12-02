//
// Created by henri on 01/12/22.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MAKE_SOLVER_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MAKE_SOLVER_H

#include "NodeBaP.h"
#include "MostInfeasibleWithSpatialBranching.h"

BranchAndBound make_solver(Model& t_rmp,
                           const Var& t_alpha,
                           Model& t_subproblem,
                           const Vector<Var>& t_integer_first_stage_variables,
                           const Vector<Var>& t_continuous_first_stage_variables) {

    BranchAndBound result;

    auto& node_strategy = result.set_node_strategy<NodeStrategies::Basic<NodeBaP>>();
    node_strategy.set_active_node_manager_strategy<ActiveNodesManagers::Basic>();
    node_strategy.set_branching_strategy<MostInfeasibleWithSpatialBranching>(
                t_integer_first_stage_variables,
                t_continuous_first_stage_variables
            );
    node_strategy.set_node_updator_strategy<NodeUpdators::ByBoundVar>();

    auto& decomposition = result.set_solution_strategy<Decomposition>();
    decomposition.set_rmp_solution_strategy<Solvers::Gurobi>(t_rmp);

    auto& column_generation = decomposition.add_generation_strategy<ColumnGeneration>();

    auto &subproblem = column_generation.add_subproblem(t_alpha);
    subproblem.set_solution_strategy<Solvers::Gurobi>(t_subproblem);
    subproblem.set_branching_scheme<ColumnGenerationBranchingSchemes::SP>();

    return result;

}

#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MAKE_SOLVER_H
