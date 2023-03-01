//
// Created by henri on 02/12/22.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_NODEBAP_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_NODEBAP_H

#include <list>
#include "backends/branch-and-bound/Nodes_Basic.h"
#include "backends/column-generation/ColumnGeneration.h"

class NodeBaP : public Nodes::Basic {
    std::list<std::pair<double, Solution::Primal>> m_active_generators;

protected:
    NodeBaP(unsigned int t_id, const NodeBaP &t_parent);

    void save_active_generators(const AbstractModel& t_strategy);
public:
    explicit NodeBaP(unsigned int t_id) : Nodes::Basic(t_id) {}

    void save_solution(const AbstractModel &t_original_model, const AbstractModel &t_relaxed_model) override {

        Basic::save_solution(t_original_model, t_relaxed_model);

        if (Nodes::Basic::primal_solution().status() == Optimal) {
            save_active_generators(t_relaxed_model);
        }
    }

    using ActiveGenerators = ConstIteratorForward<std::list<std::pair<double, Solution::Primal>>>;

    [[nodiscard]] ActiveGenerators active_generators() const { return m_active_generators; }

    [[nodiscard]] NodeBaP *create_child(unsigned int t_id) const override {
        return new NodeBaP(t_id, *this);
    }
};

NodeBaP::NodeBaP(unsigned int t_id, const NodeBaP &t_parent) : Nodes::Basic(t_id, t_parent) {

}

void NodeBaP::save_active_generators(const AbstractModel &t_strategy) {

    if (!t_strategy.backend().is<ColumnGeneration>()) {
        throw Exception("Expected solution strategy to be ColumnGeneration");
    }

    auto& column_generation = t_strategy.backend().as<ColumnGeneration>();

    if (column_generation.subproblems().size() > 1) {
        throw Exception("Not implemented for more than one subproblem.");
    }

    auto& subproblem = *column_generation.subproblems().begin();

    for (const auto& [var, primal_solution] : subproblem.present_generators()) {
        if (double value = t_strategy.get(Attr::Solution::Primal, var) ; value > 1e-4) {
            m_active_generators.emplace_back(value, primal_solution);
        }
    }

}

#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_NODEBAP_H
