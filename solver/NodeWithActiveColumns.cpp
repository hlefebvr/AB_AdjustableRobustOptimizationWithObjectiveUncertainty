//
// Created by henri on 03/04/23.
//
#include "NodeWithActiveColumns.h"

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
        if (double value = t_strategy.get_var_primal(var) ; value > TOLERANCE_FOR_ACTIVE_COLUMNS) {
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
