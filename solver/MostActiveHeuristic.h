//
// Created by henri on 03/04/23.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTACTIVEHEURISTIC_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTACTIVEHEURISTIC_H

#include "optimizers/branch-and-bound/callbacks/CallbackFactory.h"
#include "optimizers/branch-and-bound/callbacks/Callback.h"
#include "NodeWithActiveColumns.h"

class MostActiveHeuristic : public CallbackFactory<NodeWithActiveColumns> {
    std::list<Var> m_integer_branching_candidates;
    std::list<Var> m_continuous_branching_candidates;
protected:
    MostActiveHeuristic(const MostActiveHeuristic& t_src);
public:
    template<class IteratorIntegerT, class IteratorContinuousT = IteratorIntegerT>
    MostActiveHeuristic(IteratorIntegerT t_int_begin,
                        IteratorIntegerT t_int_end,
                        IteratorContinuousT t_cont_begin,
                        IteratorContinuousT t_cont_end);

    class Strategy : public Callback<NodeWithActiveColumns> {
        std::list<Var> m_integer_branching_candidates;
        std::list<Var> m_continuous_branching_candidates;

        [[nodiscard]] const Solution::Primal& most_active_generator() const;
    private:
        void operator()(BranchAndBoundEvent t_event) override;
    public:
        Strategy(std::list<Var> t_integer_branching_candidates, std::list<Var> t_continuous_branching_candidates);
    };

    Callback<NodeWithActiveColumns> *operator()() override;

    [[nodiscard]] CallbackFactory<NodeWithActiveColumns> *clone() const override;
};

template<class IteratorIntegerT, class IteratorContinuousT>
MostActiveHeuristic::MostActiveHeuristic(IteratorIntegerT t_int_begin,
                                         IteratorIntegerT t_int_end,
                                         IteratorContinuousT t_cont_begin,
                                         IteratorContinuousT t_cont_end) {

    for ( ; t_int_begin != t_int_end ; ++t_int_begin) {
        m_integer_branching_candidates.emplace_back(*t_int_begin);
    }

    for ( ; t_cont_begin != t_cont_end ; ++t_cont_begin) {
        m_continuous_branching_candidates.emplace_back(*t_cont_begin);
    }

}

#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_MOSTACTIVEHEURISTIC_H
