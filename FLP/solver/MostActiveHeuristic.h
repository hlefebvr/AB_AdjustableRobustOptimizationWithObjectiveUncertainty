//
// Created by henri on 01/03/23.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTACTIVEHEURISTIC_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTACTIVEHEURISTIC_H

#include "backends/callback/Callback.h"
#include "backends/column-generation/ColumnGeneration.h"
#include "backends/column-generation/Relaxations_DantzigWolfe.h"

template<class BackendT>
class MostActiveHeuristic : public Callback {
    std::list<Var> m_integer_variables;
    std::list<Var> m_continuous_variables;
public:
    MostActiveHeuristic(std::list<Var> t_integer_variables, std::list<Var> t_continuous_variables)
        : m_integer_variables(std::move(t_integer_variables)),
          m_continuous_variables(std::move(t_continuous_variables)) {

    }

    void execute(Event t_event) override {

        if (t_event != Event::NodeSolved) {
            return;
        }

        const auto& node = relaxation().model();

        const auto& column_generation = node.backend().template as<ColumnGeneration>();

        const Solution::Primal* most_active;
        double maximum = 0.;

        for (const auto& subproblem : column_generation.subproblems()) {
            for (const auto &[alpha, generator]: subproblem.present_generators()) {

                const double alpha_value = node.get(Attr::Solution::Primal, alpha);

                if (alpha_value > maximum) {
                    maximum = alpha_value;
                    most_active = &generator;
                }

            }
        }

        if (!most_active) {
            throw Exception("Error: no active generator found.");
        }

        auto tmp = temporary_update_session();

        for (const auto& var : m_integer_variables) {
            //const double value = std::round(most_active->get(var));
            const double value = std::ceil(node.get(Attr::Solution::Primal, var));
            tmp.set(Attr::Var::Lb, var, value);
            tmp.set(Attr::Var::Ub, var, value);
        }

        for (const auto& var : m_continuous_variables) {
            const double value = most_active->get(var);
            tmp.set(Attr::Var::Lb, var, value);
            tmp.set(Attr::Var::Ub, var, value);
        }

        tmp.reoptimize();

        auto solution = save(original_model(), Attr::Solution::Primal, node);

        submit(std::move(solution));

    }
};

#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTACTIVEHEURISTIC_H
