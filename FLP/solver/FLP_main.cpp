#include <iostream>
#include "AdjustableFLP.h"
#include "solvers.h"
/*
auto make_model(Env t_env,
                const Instance& t_instance,
                ObjectiveType t_objective_type,
                UncertaintySet t_uncertainty_set,
                double t_uncertainty_parameter) {

    const unsigned int n_facilities = t_instance.n_facilities();
    const unsigned int n_customers = t_instance.n_customers();

    Model result(t_env);
    Annotation<Ctr, unsigned int> decomposition(t_env, "decomposition", 0);

    /// VARIABLES

    // First-stage variables
    auto x = Var::array(t_env, Dim<1>(n_facilities), 0., 1., Binary, "x");
    auto q = Var::array(t_env, Dim<1>(n_facilities), 0., 1., Continuous, "q");

    // Second-stage variables
    auto y = Var::array(t_env, Dim<2>(n_facilities, n_customers), 0., 1., Continuous, "y");
    auto z = Var::array(t_env, Dim<2>(n_facilities, n_customers), 0., 1., Binary, "z");
    auto v = Var::array(t_env, Dim<1>(n_facilities), 0., Inf, Continuous, "v");
    auto r = Var::array(t_env, Dim<1>(n_facilities), 0., Inf, Continuous, "r");

    // Adversary dual variables
    auto lambda = Var(t_env, 0., Inf, Continuous, "lambda");
    auto pi = Var::array(t_env, Dim<2>(n_facilities, n_customers), 0., Inf, Continuous, "pi");

    /// CONSTRAINTS

    // Facility activation constraints
    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        Ctr node_activation(t_env, q[i] <= x[i]);
        result.add(node_activation);
    }

    // Capacity constraints
    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        Ctr capacity(t_env, idol_Sum(j, Range(n_customers), t_instance.demand(j) * y[i][j]) <= t_instance.max_capacity(i) * q[i]);
        result.add(capacity);
        capacity.set(decomposition, 0);
    }

    // Demand constraints
    for (unsigned int j = 0 ; j < n_customers ; ++j) {
        Ctr demand(t_env, idol_Sum(i, Range(n_facilities), y[i][j]) >= 1);
        result.add(demand);
        demand.set(decomposition, 0);
    }

    // Arc activation constraints
    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        for (unsigned int j = 0 ; j < n_customers ; ++j) {
            Ctr edge_activation(t_env, y[i][j] <= z[i][j]);
            result.add(edge_activation);
            edge_activation.set(decomposition, 0);
        }
    }

    // Served demand by facilities definitions
    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        auto c = result.add_ctr(v[i] == idol_Sum(j, Range(n_customers), t_instance.demand(j) * y[i][j]));
        result.set<unsigned int>(stage, c, 1);
    }

    // Congestion cost definitions
    if (t_objective_type == Convex) {
        for (unsigned int i = 0; i < n_facilities; ++i) {
            auto c = result.add_ctr(r[i] >= .75 * v[i] + .75 * v[i] * v[i]);
            result.set<unsigned int>(stage, c, 1);
        }
    } else {
        throw std::runtime_error("Not implemented.");
    }

    // Robust cost definition
    if (t_uncertainty_set == Polyhedral) {
        std::cout << "WARNING COSTS ARE WRONGLY COMPUTED" << std::endl;

        for (unsigned int i = 0; i < n_facilities; ++i) {
            for (unsigned int j = 0; j < n_customers; ++j) {
                result.add_ctr(lambda + pi[i][j] >=
                                  t_instance.transportation_fixed_cost_deviation(i ,j) * z[i][j]
                                + t_instance.per_unit_transportation_cost_deviation(i, j) * y[i][j]);
            }
        }

    } else {
        std::cout << "WARNING COSTS ARE WRONGLY COMPUTED" << std::endl;

        auto nu = result.add_vars(Dim<2>(n_facilities, n_customers), 0., Inf, Continuous, 0., "nu");
        for (unsigned int i = 0; i < n_facilities; ++i) {
            for (unsigned int j = 0; j < n_customers; ++j) {
                result.add_ctr(nu[i][j] + pi[i][j] >=
                               t_instance.transportation_fixed_cost_deviation(i ,j) * z[i][j]
                                + t_instance.per_unit_transportation_cost_deviation(i, j) * y[i][j]);
            }
        }

        result.add_ctr(idol_Sum(i, Range(n_facilities), idol_Sum(j, Range(n_customers), nu[i][j] * nu[i][j])) <= lambda * lambda);

    }

    /// OBJECTIVE FUNCTION

    Expr objective = t_uncertainty_parameter * lambda
                     + idol_Sum(i, Range(n_facilities), idol_Sum(j, Range(n_customers), pi[i][j]))
                     + idol_Sum(i, Range(n_facilities), t_instance.capacity_fixed_cost(i) * x[i])
                     + idol_Sum(i, Range(n_facilities), t_instance.per_unit_capacity_cost(i) * t_instance.max_capacity(i) * q[i])
                     + idol_Sum(i, Range(n_facilities), r[i])
                     + idol_Sum(i, Range(n_facilities),
                                idol_Sum(j, Range(n_customers),
                                     t_instance.transportation_fixed_cost_nominal(i, j) * z[i][j]
                                     + t_instance.per_unit_transportation_cost_nominal(i, j) * y[i][j]
                                ))
    ;

    result.set(Attr::Obj::Expr, std::move(objective));

    return std::make_tuple(std::move(result), x, q, decomposition);
}
*/

int main(int t_argc, const char** t_argv) {

    Logs::set_level<BranchAndBound>(Debug);
    Logs::set_color<BranchAndBound>(Blue);

    Logs::set_level<ColumnGeneration>(Debug);
    Logs::set_color<ColumnGeneration>(Yellow);

    auto instance = read_instance("/home/henri/CLionProjects/AB_AdjustableRobustOptimizationWithObjectiveUncertainty/FLP/data/instance_4_8_110__0.txt");

    const double uncertainty_parameter = 2;

    AdjustableFLP problem(instance, Convex, Polyhedral, uncertainty_parameter);

    auto& model = problem.model();

    Idol::set_optimizer<Gurobi>(model);

    model.optimize();

    std::cout << "Gurobi: " << model.get(Attr::Solution::ObjVal) << std::endl;

    Idol::set_optimizer<BranchAndPriceMIP<Gurobi>>(model, problem.decomposition());
    model.set(Param::ColumnGeneration::LogFrequency, 1);
    model.set(Param::ColumnGeneration::SmoothingFactor, 0);

    model.optimize();

    //std::cout << "Time: " << (ub.time().count() + solver.time().count()) << " s" << std::endl;
    std::cout << "ColGen: " << model.get(Attr::Solution::ObjVal) << std::endl;

    return 0;
}
