
//
// Created by henri on 03/04/23.
//
#include "MostActiveHeuristic.h"

const Solution::Primal& MostActiveHeuristic::Strategy::most_active_generator() const {

    double max_activity = 0.0;
    const Solution::Primal* argmax = nullptr;

    for (const auto& [alpha_val, generator] : node().info().active_generators()) {

        if (alpha_val > max_activity) {
            max_activity = alpha_val;
            argmax = &generator;
        }

    }

    return *argmax;
}

void MostActiveHeuristic::Strategy::operator()(BranchAndBoundEvent t_event) {

    if (t_event != InvalidSolution) {
        return;
    }

    const auto& generator = most_active_generator();

    const auto& model = relaxation();

    /**
     * Here, we are using the temporary interface which let us make temporary modifications to the node's problem.
     * We use it to fix some variables' value to the most-active column's values.
     */
    auto interface = temporary_interface();

    for (const auto& var : m_integer_branching_candidates) {
        const double current_value = node().info().primal_solution().get(var);
        const double fixation = std::ceil(current_value);
        interface.set(Attr::Var::Lb, var, fixation);
        interface.set(Attr::Var::Ub, var, fixation);
    }

    for (const auto& var : m_continuous_branching_candidates) {
        const double current_value = node().info().primal_solution().get(var);
        const double fixation = current_value;
        interface.set(Attr::Var::Lb, var, fixation);
        interface.set(Attr::Var::Ub, var, fixation);
    }

    interface.reoptimize();

    auto* info = new NodeWithActiveColumns();
    info->save(original_model(), relaxation());

    if (info->status() != Optimal) {
        delete info;
        return;
    }

    submit_heuristic_solution(info);

}

MostActiveHeuristic::Strategy::Strategy(std::list<Var> t_integer_branching_candidates,
                                        std::list<Var> t_continuous_branching_candidates)
    : m_integer_branching_candidates(std::move(t_integer_branching_candidates)),
      m_continuous_branching_candidates(std::move(t_continuous_branching_candidates)) {

}

Callback<NodeWithActiveColumns> *MostActiveHeuristic::operator()() {
    return new Strategy(m_integer_branching_candidates, m_continuous_branching_candidates);
}

CallbackFactory<NodeWithActiveColumns> *MostActiveHeuristic::clone() const {
    return new MostActiveHeuristic(*this);
}

MostActiveHeuristic::MostActiveHeuristic(const MostActiveHeuristic &t_src)
    : m_integer_branching_candidates(t_src.m_integer_branching_candidates),
      m_continuous_branching_candidates(t_src.m_continuous_branching_candidates) {

}
