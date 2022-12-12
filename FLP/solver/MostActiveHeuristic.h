//
// Created by henri on 12/12/22.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTACTIVEHEURISTIC_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTACTIVEHEURISTIC_H

#include <modeling.h>
#include <algorithms.h>
#include "NodeBaP.h"

/**
 * WARNING: this heuristic is dedicated to FLP only (in particular, expects, q <= x part of the model with x binary and q continuous)
 */
class MostActiveHeuristic : public BranchAndBound::Callback {
    const Vector<Var, 1>& m_integer_branching_candidates;
    const Vector<Var, 1>& m_continuous_branching_candidates;
public:
    MostActiveHeuristic(const Vector<Var, 1>& t_integer_branching_candidates, const Vector<Var, 1>& t_continuous_branching_candidates)
        : m_integer_branching_candidates(t_integer_branching_candidates),
          m_continuous_branching_candidates(t_continuous_branching_candidates) {}

    void execute(Context &t_ctx) override {

        if (t_ctx.event() != RelaxationSolved) { return; }

        // If gap <= 3%, skip

        const auto& node = advanced(t_ctx).node();
        const auto& node_bap = dynamic_cast<const NodeBaP&>(node);

        const Solution::Primal* most_active;
        double maximum = 0.;

        for (const auto& [alpha_value, generator] : node_bap.active_generators()) {
            if (alpha_value > maximum) {
                maximum = alpha_value;
                most_active = &generator;
            }
        }

        if (!most_active) {
            throw Exception("Error: no active generator found.");
        }

        if (maximum >= 1 - 1e-4) { return; }

        std::list<std::pair<Var, double>> fixations;

        for (unsigned int i = 0, n = m_integer_branching_candidates.size() ; i < n ; ++i) {

            const auto& x_i = m_integer_branching_candidates[i];
            const auto& q_i = m_continuous_branching_candidates[i];
            double q_i_value = most_active->get(q_i);

            fixations.emplace_back( q_i, q_i_value );
            fixations.emplace_back( x_i, q_i_value > 1e-4 ? 1 : 0 );

        }

        advanced(t_ctx).fix_variables(fixations);
        advanced(t_ctx).resolve();

        auto solution = advanced(t_ctx).primal_solution();

        if (solution.status() == Optimal) {
            t_ctx.submit_solution( std::move(solution) );
        }

    }
};

#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTACTIVEHEURISTIC_H
