//
// Created by henri on 01/12/22.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTINFEASIBLEWITHSPATIALBRANCHING_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTINFEASIBLEWITHSPATIALBRANCHING_H

#include "modeling.h"
#include "optimizers/branch-and-bound/branching-rules/impls/MostInfeasbile.h"
#include "optimizers/branch-and-bound/branching-rules/factories/BranchingRuleFactory.h"
#include "tolerances.h"

namespace BranchingRules {
    template<class NodeInfoT> class MostInfeasibleWithSpatialBranching;
}

template<class NodeInfoT>
class BranchingRules::MostInfeasibleWithSpatialBranching : public BranchingRules::MostInfeasible<NodeInfoT> {
    std::list<Var> m_continuous_branching_candidates;

    double most_diverse_score(const Var& t_var, const Node<NodeInfoT>& t_node);
    void select_continuous_variable_for_branching(const Node<NodeInfoT>& t_node);
    std::list<NodeInfoT *> create_child_nodes_continuous(const Node<NodeInfoT> &t_node);
    std::list<NodeInfoT*> create_child_nodes_by_bound_for_continuous_variable(const Node<NodeInfoT> &t_node,
                                                                          const Var &t_var,
                                                                          double t_value);
public:
    MostInfeasibleWithSpatialBranching(const Model& t_model,
                                       std::list<Var> t_integer_branching_candidates,
                                       std::list<Var> t_continuous_branching_candidates)
        : BranchingRules::MostInfeasible<NodeInfoT>(t_model, std::move(t_integer_branching_candidates)),
          m_continuous_branching_candidates(std::move(t_continuous_branching_candidates)) {}


    bool is_valid(const Node<NodeInfoT> &t_node) const override;

    std::list<NodeInfoT*> create_child_nodes(const Node<NodeInfoT> &t_node) override;
};

template<class NodeInfoT>
bool BranchingRules::MostInfeasibleWithSpatialBranching<NodeInfoT>::is_valid(const Node<NodeInfoT> &t_node) const {

    if (!BranchingRules::MostInfeasible<NodeInfoT>::is_valid(t_node)) {
        return false;
    }

    const auto& solution = t_node.info().primal_solution();
    for (const auto& [alpha_value, generator] : t_node.info().active_generators()) {

        for (const Var& var : m_continuous_branching_candidates) {
            if (equals(solution.get(var), generator.get(var), TOLERANCE_FOR_CONTINUOUS_VARIABLES)) {
                continue;
            }
            //std::cout << "Continuous infeasible " << var << ", " << std::setprecision(5) << " value = " << solution.get(var) << " vs generator = " << generator.get(var) << std::endl;
            return false;
        }

    }

    return true;
}

template<class NodeInfoT>
std::list<NodeInfoT *>
BranchingRules::MostInfeasibleWithSpatialBranching<NodeInfoT>::create_child_nodes(const Node<NodeInfoT> &t_node) {

    this->reset_variable_selected_for_branching();

    this->select_integer_variable_for_branching(t_node);

    if (!this->has_variable_selected_for_branching()) {
        return create_child_nodes_continuous(t_node);
    }

    const auto [variable, score] = this->variable_selected_for_branching();

    if (score <= ToleranceForIntegrality) {
        throw Exception("Maximum infeasibility is less than ToleranceForIntegrality.");
    }

    const double value = t_node.info().primal_solution().get(variable);

    return this->create_child_nodes_by_bound_for_integer_variable(t_node, variable, value);

}

template<class NodeInfoT>
std::list<NodeInfoT *>
BranchingRules::MostInfeasibleWithSpatialBranching<NodeInfoT>::create_child_nodes_by_bound_for_continuous_variable(
        const Node<NodeInfoT> &t_node,
        const Var &t_var,
        double t_value) {
    return this->create_child_nodes_by_bound(t_node, t_var, t_value, t_value, t_value);
}

template<class NodeInfoT>
std::list<NodeInfoT *>
BranchingRules::MostInfeasibleWithSpatialBranching<NodeInfoT>::create_child_nodes_continuous(const Node<NodeInfoT> &t_node) {

    select_continuous_variable_for_branching(t_node);

    if (!this->has_variable_selected_for_branching()) {
        throw Exception("No variable selected for branching.");
    }

    const auto [variable, score] = this->variable_selected_for_branching();

    if (score <= TOLERANCE_FOR_CONTINUOUS_VARIABLES) {
        throw Exception("Maximum infeasibility is less than TOLERANCE_FOR_CONTINUOUS_VARIABLES.");
    }

    const double value = t_node.info().primal_solution().get(variable);

    return create_child_nodes_by_bound_for_continuous_variable(t_node, variable, value);

}

template<class NodeInfoT>
void BranchingRules::MostInfeasibleWithSpatialBranching<NodeInfoT>::select_continuous_variable_for_branching(const Node<NodeInfoT> &t_node) {

    this->select_variable_for_branching(
            m_continuous_branching_candidates.begin(),
            m_continuous_branching_candidates.end(),
            [&](const Var& t_var){ return most_diverse_score(t_var, t_node); }
    );

}

template<class NodeInfoT>
double BranchingRules::MostInfeasibleWithSpatialBranching<NodeInfoT>::most_diverse_score(const Var &t_var,
                                                                                         const Node<NodeInfoT> &t_node) {

    unsigned int n_different_generators = 0;
    double max_score = -Inf;

    const auto& solution = t_node.info().primal_solution();
    for (const auto& [alpha_value, generator] : t_node.info().active_generators()) {

        const double score = std::abs( solution.get(t_var) - generator.get(t_var) );

        if ( score > TOLERANCE_FOR_CONTINUOUS_VARIABLES ) {
            if (score > max_score) {
                max_score = score;
            }
            ++n_different_generators;
        }
    }

    if (n_different_generators == 0) {
        return -Inf;
    }

    return max_score;
    // return (double) n_different_generators;
}

class MostInfeasibleWithSpatialBranching {
    std::list<Var> m_explicit_integer_branching_candidates;
    std::list<Var> m_explicit_continuous_branching_candidates;
public:
    template<class IteratorIntegerT, class IteratorContinuousT = IteratorIntegerT>
    MostInfeasibleWithSpatialBranching(IteratorIntegerT t_int_begin,
                                       IteratorIntegerT t_int_end,
                                       IteratorContinuousT t_cont_begin,
                                       IteratorContinuousT t_cont_end) {

        for ( ; t_int_begin != t_int_end ; ++t_int_begin) {
            m_explicit_integer_branching_candidates.emplace_back(*t_int_begin);
        }

        for ( ; t_cont_begin != t_cont_end ; ++t_cont_begin) {
            m_explicit_continuous_branching_candidates.emplace_back(*t_cont_begin);
        }

    }

    template<class NodeT>
    class Strategy : public BranchingRuleFactory<NodeT> {
        std::list<Var> m_explicit_integer_branching_candidates;
        std::list<Var> m_explicit_continuous_branching_candidates;
    public:
        explicit Strategy(const MostInfeasibleWithSpatialBranching& t_parent)
            : m_explicit_integer_branching_candidates(t_parent.m_explicit_integer_branching_candidates),
              m_explicit_continuous_branching_candidates(t_parent.m_explicit_continuous_branching_candidates) {}

        BranchingRules::MostInfeasibleWithSpatialBranching<NodeT>* operator()(const Model& t_model) const override {
            return new BranchingRules::MostInfeasibleWithSpatialBranching<NodeT>(
                    t_model,
                    m_explicit_integer_branching_candidates,
                    m_explicit_continuous_branching_candidates);
        }

        Strategy *clone() const override { return new Strategy(*this); }
    };

};


#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTINFEASIBLEWITHSPATIALBRANCHING_H
