//
// Created by henri on 02/12/22.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_NODEBAP_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_NODEBAP_H

#include <list>
#include "algorithms.h"

class NodeBaP : public Nodes::Basic {
    std::list<std::pair<double, Solution::Primal>> m_active_generators;

protected:
    NodeBaP(unsigned int t_id, const NodeBaP &t_parent);

    void save_active_columns(const Algorithm& t_strategy);
public:
    explicit NodeBaP(unsigned int t_id) : Nodes::Basic(t_id) {}

    void save_solution(const Algorithm &t_strategy) override {

        Basic::save_solution(t_strategy);

        if (Nodes::Basic::primal_solution().status() == Optimal) {
            save_active_columns(t_strategy);
        }
    }

    using ActiveGenerators = ConstIteratorForward<std::list<std::pair<double, Solution::Primal>>>;

    ActiveGenerators active_generators() const { return m_active_generators; }

    NodeBaP *create_child(unsigned int t_id) const override {
        return new NodeBaP(t_id, *this);
    }
};

NodeBaP::NodeBaP(unsigned int t_id, const NodeBaP &t_parent) : Nodes::Basic(t_id, t_parent) {

}

void NodeBaP::save_active_columns(const Algorithm &t_strategy) {

    if (!t_strategy.is<Decomposition>() || !t_strategy.as<Decomposition>().begin()->is<ColumnGeneration>()) {
        throw Exception("Expected solution strategy to be Decomposition > ColumnGeneration.");
    }

    auto& column_generation = t_strategy.as<Decomposition>().begin()->as<ColumnGeneration>();

    if (column_generation.subproblems().size() > 1) {
        throw Exception("Not implemented for more than one subproblem.");
    }

    auto& subproblem = *column_generation.subproblems().begin();
    auto& solution = primal_solution();

    for (const auto& [var, primal_solution] : subproblem.currently_present_variables()) {
        if (double value = solution.get(var) ; value > 1e-4) {
            m_active_generators.emplace_back(value, primal_solution);
        }
    }

}

#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_NODEBAP_H
