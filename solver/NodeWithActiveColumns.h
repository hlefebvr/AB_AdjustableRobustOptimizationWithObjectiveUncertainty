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

#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_NODEWITHACTIVECOLUMNS_H
