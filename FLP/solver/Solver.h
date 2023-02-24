//
// Created by henri on 24/02/23.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_SOLVER_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_SOLVER_H

#include "backends/branch-and-bound/BranchAndBound.h"
#include "backends/column-generation/Relaxations_DantzigWolfe.h"
#include "backends/branch-and-bound/NodeStrategies_Basic.h"
#include "backends/branch-and-bound/ActiveNodesManagers_Basic.h"
#include "backends/branch-and-bound/BranchingStrategies_MostInfeasible.h"
#include "backends/branch-and-bound/NodeUpdators_ByBoundVar.h"
#include "backends/branch-and-bound/Nodes_Basic.h"
#include "backends/column-generation/ColumnGeneration.h"
#include "MostInfeasibleWithSpatialBranching.h"
#include "NodeBaP.h"

template<class MasterProblemBackendT, class SubProblemBackendT = MasterProblemBackendT>
class Solver : public BranchAndBound {
public:
    explicit Solver(const AbstractModel& t_original_formulation,
                    const Annotation<Ctr, unsigned int>& t_annotation)
            : BranchAndBound(t_original_formulation) {

        auto& relaxation = set_relaxation<Relaxations::DantzigWolfe>(t_annotation);
        relaxation.build();

        auto& column_generation = Idol::set_optimizer<ColumnGeneration>(relaxation.model());

        column_generation.template set_master_backend<MasterProblemBackendT>();
        for (unsigned int i = 0, n = relaxation.model().n_blocks() ; i < n ; ++i) {
            column_generation.template set_subproblem_backend<SubProblemBackendT>(i);
        }

        std::list<Var> integer_variables;
        std::list<Var> continuous_variables;

        for (const auto& var : t_original_formulation.vars()) {

            if (var.name().front() == 'x') {
                integer_variables.emplace_back(var);
            } else if (var.name().front() == 'q') {
                continuous_variables.emplace_back(var);
            }

        }

        auto& nodes_manager = set_node_strategy<NodeStrategies::Basic<NodeBaP>>();
        nodes_manager.template set_active_node_manager<ActiveNodesManagers::Basic>();
        nodes_manager.template set_branching_strategy<MostInfeasibleWithSpatialBranching>(std::move(integer_variables), std::move(continuous_variables));
        nodes_manager.template set_node_updator<NodeUpdators::ByBoundVar>();
    }
};

#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_SOLVER_H
