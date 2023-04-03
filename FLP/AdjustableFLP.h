//
// Created by henri on 20/02/23.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_ADJUSTABLEFLP_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_ADJUSTABLEFLP_H

#include "Instance.h"
#include "modeling.h"

enum UncertaintySet { Polyhedral, Ellipsoidal };
enum ObjectiveType { Convex, Linearized };

static std::ostream& operator<<(std::ostream& t_os, UncertaintySet t_uncertainty_set) {
    switch (t_uncertainty_set) {
        case Polyhedral: return t_os << "Polyhedral";
        case Ellipsoidal: return t_os << "Ellipsoidal";
        default:;
    }
    throw std::runtime_error("enum out of bounds.");
}

static std::ostream& operator<<(std::ostream& t_os, ObjectiveType t_objective_type) {
    switch (t_objective_type) {
        case Convex: return t_os << "Convex";
        case Linearized: return t_os << "Linearized";
        default:;
    }
    throw std::runtime_error("enum out of bounds.");
}

class AdjustableFLP {
    const Instance& m_instance;
    UncertaintySet m_uncertainty_set;
    ObjectiveType m_objective_type;
    double m_uncertainty_parameter;
    Env m_env;
    Model m_model;
    Annotation<Ctr, unsigned int> m_decomposition;

    // First stage variables
    Vector<Var, 1> m_x;
    Vector<Var, 1> m_q;

    // Second stage variables
    Vector<Var, 2> m_y;
    Vector<Var, 2> m_z;
    Vector<Var, 1> m_v;
    Vector<Var, 1> m_r;

    // Adversarial variables
    Vector<Var> m_lambda;
    Vector<Var, 2> m_pi;
protected:
    void create_first_stage_variables_x();
    void create_first_stage_variables_q();

    void create_second_stage_variables_y();
    void create_second_stage_variables_z();
    void create_second_stage_variables_v();
    void create_second_stage_variables_r();

    void create_adversarial_variables_lambda();
    void create_adversarial_variables_pi();

    void create_facility_activation_constraints();
    void create_facility_capacity_constraints();
    void create_customer_demand_constraints();
    void create_connection_activation_constraints();
    void create_served_demand_constraints();
    void create_congestion_cost_constraints();
    void create_robust_counterpart_constraints();

    void create_objective();
public:
    AdjustableFLP(const Instance& t_instance, ObjectiveType t_objective_type, UncertaintySet t_uncertainty_set, double t_uncertainty_parameter);

    [[nodiscard]] const Model& model() const { return m_model; }

    Model& model() { return m_model; }

    [[nodiscard]] const Annotation<Ctr, unsigned int>& decomposition() const { return m_decomposition; }

    [[nodiscard]] const auto& x() const { return m_x; }
    [[nodiscard]] const auto& q() const { return m_q; }
};

#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_ADJUSTABLEFLP_H
