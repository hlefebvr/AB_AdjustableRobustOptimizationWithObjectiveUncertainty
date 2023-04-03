//
// Created by henri on 02/12/22.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_NODEWITHACTIVECOLUMNS_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_NODEWITHACTIVECOLUMNS_H

#include <list>
#include "optimizers/branch-and-bound/nodes/NodeInfo.h"
#include "optimizers/column-generation/ColumnGeneration.h"
#include "optimizers/branch-and-bound/nodes/NodeUpdatorByBound.h"
#include "tolerances.h"

class NodeWithActiveColumns : public NodeInfo {
    std::optional<std::list<std::pair<double, Solution::Primal>>> m_active_generators;

    void save_active_generators(const Model& t_strategy);
protected:
    NodeWithActiveColumns(const NodeWithActiveColumns& t_src);
public:
    NodeWithActiveColumns() = default;

    void save(const Model &t_original_formulation, const Model &t_model) override;

    [[nodiscard]] NodeWithActiveColumns *create_child() const override;

    using ActiveGenerators = ConstIteratorForward<std::list<std::pair<double, Solution::Primal>>>;

    [[nodiscard]] ActiveGenerators active_generators() const { return m_active_generators.value(); }

    static NodeUpdator<NodeWithActiveColumns>* create_updator(Model& t_model);
};

void NodeWithActiveColumns::save(const Model &t_original_formulation, const Model &t_model) {

    NodeInfo::save(t_original_formulation, t_model);

    if (status() == Optimal) {
        save_active_generators(t_model);
    }

}

void NodeWithActiveColumns::save_active_generators(const Model &t_strategy) {

    if (!t_strategy.optimizer().is<Optimizers::ColumnGeneration>()) {
        throw Exception("Expected optimizer to be ColumnGeneration");
    }

    auto& column_generation = t_strategy.optimizer().as<Optimizers::ColumnGeneration>();

    if (column_generation.subproblems().size() > 1) {
        throw Exception("Not implemented for more than one sub-problem.");
    }

    auto& subproblem = *column_generation.subproblems().begin();

    std::list<std::pair<double, Solution::Primal>> active_generators;

    for (const auto& [var, primal_solution] : subproblem.present_generators()) {
        if (double value = t_strategy.get(Attr::Solution::Primal, var) ; value > TOLERANCE_FOR_ACTIVE_COLUMNS) {
            active_generators.emplace_back(value, primal_solution);
        }
    }

    m_active_generators = std::make_optional(std::move(active_generators));

}

NodeWithActiveColumns *NodeWithActiveColumns::create_child() const {
    return new NodeWithActiveColumns(*this);
}

NodeUpdator<NodeWithActiveColumns> *NodeWithActiveColumns::create_updator(Model &t_model) {
    return new NodeUpdatorByBound<NodeWithActiveColumns>(t_model);
}

NodeWithActiveColumns::NodeWithActiveColumns(const NodeWithActiveColumns &t_src) : NodeInfo(t_src) {

}

#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_NODEWITHACTIVECOLUMNS_H
