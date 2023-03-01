//
// Created by henri on 01/12/22.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTINFEASIBLEWITHSPATIALBRANCHING_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTINFEASIBLEWITHSPATIALBRANCHING_H

#include "modeling.h"
#include "backends/branch-and-bound/BranchingStrategies_MostInfeasible.h"

#define TOLERANCE_FOR_CONTINUOUS 1e-3

class MostInfeasibleWithSpatialBranching {
public:
    template<class NodeT> class Strategy;
};

template<class NodeT>
class MostInfeasibleWithSpatialBranching::Strategy : public BranchingStrategies::MostInfeasible::Strategy<NodeT> {
    std::list<Var> m_continuous_branching_candidates;
protected:
    static double most_diverse_score(const Var& t_var, const NodeT& t_node);
    std::list<NodeT *> create_child_nodes_continuous(const NodeT &t_node, const std::function<unsigned int()> &t_id_provider);
    void select_continuous_variable_for_branching(const NodeT& t_node);
    std::list<NodeT*> create_child_nodes_by_bound_for_continuous_variable(const NodeT &t_node,
                                                             const std::function<unsigned int()>& t_id_provider,
                                                             const Var &t_var,
                                                             double t_value);
public:
    Strategy(std::list<Var> t_integer_branching_candidates,
             std::list<Var> t_continuous_branching_candidates);

    bool is_valid(const NodeT &t_node) const override;

    std::list<NodeT *> create_child_nodes(const NodeT &t_node, const std::function<unsigned int()> &t_id_provider) override;
};

template<class NodeT>
double MostInfeasibleWithSpatialBranching::Strategy<NodeT>::most_diverse_score(const Var &t_var, const NodeT &t_node) {

    unsigned int n_different_generators = 0;
    double max_score = -1;

    const auto& solution = t_node.primal_solution();
    for (const auto& [alpha_value, generator] : t_node.active_generators()) {

        const double score = std::abs( alpha_value * solution.get(t_var) - alpha_value * generator.get(t_var) );

        if ( score > TOLERANCE_FOR_CONTINUOUS ) {
            if (score > max_score) {
                max_score = score;
            }
            ++n_different_generators;
        }
    }

    if (n_different_generators == 0) {
        return -1.;
    }

    return max_score;

    return (double) n_different_generators;

}

template<class NodeT>
void
MostInfeasibleWithSpatialBranching::Strategy<NodeT>::select_continuous_variable_for_branching(const NodeT &t_node) {

    this->select_variable_for_branching(
            m_continuous_branching_candidates.begin(),
            m_continuous_branching_candidates.end(),
            [&](const Var& t_var){ return most_diverse_score(t_var, t_node); }
    );

}

template<class NodeT>
std::list<NodeT*> MostInfeasibleWithSpatialBranching::Strategy<NodeT>::create_child_nodes_by_bound_for_continuous_variable(
        const NodeT &t_node,
        const std::function<unsigned int()>& t_id_provider,
        const Var &t_var,
        double t_value) {

    return this->create_child_nodes_by_bound(t_node, t_id_provider, t_var, t_value, t_value, t_value);

}

template<class NodeT>
bool MostInfeasibleWithSpatialBranching::Strategy<NodeT>::is_valid(const NodeT &t_node) const {

    if (!BranchingStrategies::MostInfeasible::Strategy<NodeT>::Strategy::is_valid(t_node)) {
        return false;
    }

    const auto& solution = t_node.primal_solution();
    for (const auto& [alpha_value, generator] : t_node.active_generators()) {

        for (const Var& var : m_continuous_branching_candidates) {
            if ( std::abs( alpha_value * solution.get(var) - alpha_value * generator.get(var) ) > TOLERANCE_FOR_CONTINUOUS ) {
                //std::cout << "Continuous infeasible " << var << ", " << solution.get(var) << " vs " << generator.get(var) << std::endl;
                return false;
            }
        }
    }

    return true;
}

template<class NodeT>
std::list<NodeT *> MostInfeasibleWithSpatialBranching::Strategy<NodeT>::create_child_nodes(const NodeT &t_node,
                                                                                           const std::function<unsigned int()> &t_id_provider) {

    this->reset_variable_selected_for_branching();

    this->select_integer_variable_for_branching(t_node);

    if (!this->has_variable_selected_for_branching()) {
        return create_child_nodes_continuous(t_node, t_id_provider);
    }

    const auto [variable, score] = this->variable_selected_for_branching();

    if (score <= ToleranceForIntegrality) {
        throw Exception("Maximum infeasibility is less than ToleranceForIntegrality.");
    }

    const double value = t_node.primal_solution().get(variable);

    return this->create_child_nodes_by_bound_for_integer_variable(t_node, t_id_provider, variable, value);


}

template<class NodeT>
std::list<NodeT *>
MostInfeasibleWithSpatialBranching::Strategy<NodeT>::create_child_nodes_continuous(const NodeT &t_node,
                                                                                   const std::function<unsigned int()> &t_id_provider) {

    select_continuous_variable_for_branching(t_node);

    if (!this->has_variable_selected_for_branching()) {
        throw Exception("No variable selected for branching.");
    }

    const auto [variable, score] = this->variable_selected_for_branching();

    if (score <= 1e-4) {
        throw Exception("Maximum infeasibility is less than 1e-4.");
    }

    const double value = t_node.primal_solution().get(variable);

    return create_child_nodes_by_bound_for_continuous_variable(t_node, t_id_provider, variable, value);

}

template<class NodeT>
MostInfeasibleWithSpatialBranching::Strategy<NodeT>::Strategy(std::list<Var> t_integer_branching_candidates,
                                                              std::list<Var> t_continuous_branching_candidates)
      : BranchingStrategies::MostInfeasible::Strategy<NodeT>::Strategy(std::move(t_integer_branching_candidates)),
        m_continuous_branching_candidates(std::move(t_continuous_branching_candidates)) {

}


#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTINFEASIBLEWITHSPATIALBRANCHING_H
