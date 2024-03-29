//
// Created by henri on 20/02/23.
//
#include "AdjustableFLP.h"

AdjustableFLP::AdjustableFLP(const Instance &t_instance, ObjectiveType t_objective_type,
                             UncertaintySet t_uncertainty_set, double t_uncertainty_parameter)
                             : m_instance(t_instance),
                               m_model(m_env),
                               m_decomposition(m_env, "decomposition", MasterId),
                               m_objective_type(t_objective_type),
                               m_uncertainty_set(t_uncertainty_set),
                               m_uncertainty_parameter(t_uncertainty_parameter) {

    create_first_stage_variables_x();
    create_first_stage_variables_q();

    create_second_stage_variables_y();
    create_second_stage_variables_z();
    create_second_stage_variables_v();
    create_second_stage_variables_r();

    create_adversarial_variables_lambda();
    create_adversarial_variables_pi();

    create_facility_activation_constraints();
    create_facility_capacity_constraints();
    create_customer_demand_constraints();
    create_connection_activation_constraints();
    create_served_demand_constraints();
    create_congestion_cost_constraints();
    create_robust_counterpart_constraints();

    create_objective();
}

void AdjustableFLP::create_first_stage_variables_x() {
    m_x = m_model.add_vars(Dim<1>(m_instance.n_facilities()), 0., 1., Binary, "x");
}

void AdjustableFLP::create_first_stage_variables_q() {
    m_q = m_model.add_vars(Dim<1>(m_instance.n_facilities()), 0., 1., Continuous, "q");
}

void AdjustableFLP::create_second_stage_variables_y() {
    const unsigned int n_facilities = m_instance.n_facilities();
    const unsigned int n_customers = m_instance.n_customers();

    m_y = m_model.add_vars(Dim<2>(n_facilities, n_customers), 0., 1., Continuous, "y");
}

void AdjustableFLP::create_second_stage_variables_z() {
    const unsigned int n_facilities = m_instance.n_facilities();
    const unsigned int n_customers = m_instance.n_customers();

    m_z = m_model.add_vars(Dim<2>(n_facilities, n_customers), 0., 1., Binary, "z");
}

void AdjustableFLP::create_second_stage_variables_v() {
    const unsigned int n_facilities = m_instance.n_facilities();
    const unsigned int n_customers = m_instance.n_customers();

    double sum_demands = 0.;
    for (unsigned int j = 0 ; j < n_customers ; ++j) {
        sum_demands += m_instance.demand(j);
    }

    m_v = m_model.add_vars(Dim<1>(n_facilities), 0., sum_demands, Continuous, "v");
}

void AdjustableFLP::create_second_stage_variables_r() {
    const unsigned int n_facilities = m_instance.n_facilities();
    const unsigned int n_customers = m_instance.n_customers();

    double max = 0.;
    for (unsigned int j = 0 ; j < n_facilities ; ++j) {
        double congestion = .75 * m_instance.max_capacity(j) * (1 + m_instance.max_capacity(j));
        if (congestion > max) {
            max = congestion;
        }
    }

    m_r = m_model.add_vars(Dim<1>(m_instance.n_facilities()), 0., max, Continuous, "r");
}

void AdjustableFLP::create_adversarial_variables_lambda() {
    m_lambda = { Var(m_env, 0., Inf, Continuous, "lambda") };
    m_model.add(m_lambda.front());
}

void AdjustableFLP::create_adversarial_variables_pi() {
    m_pi = m_model.add_vars(Dim<2>(m_instance.n_facilities(), m_instance.n_customers()), 0., Inf, Continuous, "pi");
}

void AdjustableFLP::create_facility_activation_constraints() {
    const unsigned int n_facilities = m_instance.n_facilities();

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        Ctr node_activation(m_env, m_q[i] <= m_x[i], "facility_activation_" + std::to_string(i));
        m_model.add(node_activation);
        //node_activation.set(m_decomposition, 0);
    }

}

void AdjustableFLP::create_facility_capacity_constraints() {
    const unsigned int n_facilities = m_instance.n_facilities();
    const unsigned int n_customers = m_instance.n_customers();

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        Ctr capacity(m_env, idol_Sum(j, Range(n_customers), m_instance.demand(j) * m_y[i][j]) <= m_instance.max_capacity(i) * m_q[i], "facility_capacity_" + std::to_string(i));
        m_model.add(capacity);
        capacity.set(m_decomposition, 0);
    }
}

void AdjustableFLP::create_customer_demand_constraints() {
    const unsigned int n_facilities = m_instance.n_facilities();
    const unsigned int n_customers = m_instance.n_customers();

    for (unsigned int j = 0 ; j < n_customers ; ++j) {
        Ctr demand(m_env, idol_Sum(i, Range(n_facilities), m_y[i][j]) >= 1, "customer_demand_" + std::to_string(j));
        m_model.add(demand);
        demand.set(m_decomposition, 0);
    }

}

void AdjustableFLP::create_connection_activation_constraints() {
    const unsigned int n_facilities = m_instance.n_facilities();
    const unsigned int n_customers = m_instance.n_customers();

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        for (unsigned int j = 0 ; j < n_customers ; ++j) {
            Ctr edge_activation(m_env, m_y[i][j] <= m_z[i][j], "connection_activation_" + std::to_string(i) + "_" + std::to_string(j));
            m_model.add(edge_activation);
            edge_activation.set(m_decomposition, 0);
        }
    }
}

void AdjustableFLP::create_served_demand_constraints() {
    const unsigned int n_facilities = m_instance.n_facilities();
    const unsigned int n_customers = m_instance.n_customers();

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        Ctr served_demand(m_env, m_v[i] == idol_Sum(j, Range(n_customers), m_instance.demand(j) * m_y[i][j]), "served_demand_" + std::to_string(i));
        m_model.add(served_demand);
        served_demand.set(m_decomposition, 0);
    }
}

void AdjustableFLP::create_congestion_cost_constraints() {
    const unsigned int n_facilities = m_instance.n_facilities();
    const unsigned int n_customers = m_instance.n_customers();

    if (m_objective_type == Convex) {

        for (unsigned int i = 0; i < n_facilities; ++i) {
            Ctr congestion_cost(m_env, m_r[i] >= .75 * m_v[i] + .75 * m_v[i] * m_v[i], "congestion_" + std::to_string(i));
            m_model.add(congestion_cost);
            congestion_cost.set(m_decomposition, 0);
        }

    } else {

        const unsigned int L = 10;

        for (unsigned int i = 0 ; i < n_facilities ; ++i) {

            const double max = m_instance.max_capacity(i);

            for (unsigned int l = 0 ; l < L ; ++l) {

                const double v_il = ((double) l) * max / ((double) L);

                Ctr congestion_cost(m_env, m_r[i] >= -.75 * v_il * v_il + .75 * ( 1 + 2 * v_il ) * m_v[i], "congestion_" + std::to_string(i));
                m_model.add(congestion_cost);
                congestion_cost.set(m_decomposition, 0);

            }


        }

    }
}

void AdjustableFLP::create_robust_counterpart_constraints() {
    const unsigned int n_facilities = m_instance.n_facilities();
    const unsigned int n_customers = m_instance.n_customers();

    if (m_uncertainty_set == Polyhedral) {
        //std::cout << "WARNING COSTS ARE WRONGLY COMPUTED" << std::endl;

        for (unsigned int i = 0; i < n_facilities; ++i) {
            for (unsigned int j = 0; j < n_customers; ++j) {
                Ctr robust_counterpart(m_env, m_lambda[0] + m_pi[i][j] >=
                               m_instance.transportation_fixed_cost_deviation(i ,j) * m_z[i][j]
                               + m_instance.per_unit_transportation_cost_deviation(i, j) * m_instance.demand(j) * m_y[i][j],
                               "robust_counterpart_" + std::to_string(i) + "_" + std::to_string(j));
                m_model.add(robust_counterpart);
            }
        }

    } else {
        //std::cout << "WARNING COSTS ARE WRONGLY COMPUTED" << std::endl;

        auto nu = m_model.add_vars(Dim<2>(n_facilities, n_customers), 0., Inf, Continuous, "nu");

        for (unsigned int i = 0; i < n_facilities; ++i) {
            for (unsigned int j = 0; j < n_customers; ++j) {
                Ctr robust_counterpart(m_env, nu[i][j] + m_pi[i][j] >=
                               m_instance.transportation_fixed_cost_deviation(i, j) * m_z[i][j]
                               + m_instance.per_unit_transportation_cost_deviation(i, j) * m_instance.demand(j) * m_y[i][j],
                                "robust_counterpart" + std::to_string(i) + "_" + std::to_string(j));
                m_model.add(robust_counterpart);
            }
        }

        Ctr robust_counterpart_2(m_env, idol_Sum(i, Range(n_facilities), idol_Sum(j, Range(n_customers), nu[i][j] * nu[i][j])) <= m_lambda[0] * m_lambda[0], "robust_counterpart_2");
        m_model.add(robust_counterpart_2);

    }
}

void AdjustableFLP::create_objective() {
    const unsigned int n_facilities = m_instance.n_facilities();
    const unsigned int n_customers = m_instance.n_customers();

    Expr objective = m_uncertainty_parameter * m_lambda[0]
                     + idol_Sum(i, Range(n_facilities), idol_Sum(j, Range(n_customers), m_pi[i][j]))
                     + idol_Sum(i, Range(n_facilities), m_instance.capacity_fixed_cost(i) * m_x[i])
                     + idol_Sum(i, Range(n_facilities), m_instance.per_unit_capacity_cost(i) * m_instance.max_capacity(i) * m_q[i])
                     + idol_Sum(i, Range(n_facilities), m_r[i])
                     + idol_Sum(i, Range(n_facilities),
                                idol_Sum(j, Range(n_customers),
                                         m_instance.transportation_fixed_cost_nominal(i, j) * m_z[i][j]
                                         + m_instance.per_unit_transportation_cost_nominal(i, j) * m_y[i][j]
                                ))
    ;

    m_model.set_obj_expr(std::move(objective));
}

