#include <iostream>
#include "../Instance.h"
#include "make_solver.h"
#include "MostActiveHeuristic.h"
#include <modeling.h>
#include <reformulations/Reformulations_DantzigWolfe.h>

enum UncertaintySet { Polyhedral, Ellipsoidal };
enum ObjectiveType { Convex, Linearized };

std::tuple<Model, Vector<Var, 1>, Vector<Var, 1>, UserAttr> make_single_stage_model(const Instance& t_instance,
                             ObjectiveType t_objective_type,
                             UncertaintySet t_uncertainty_set,
                             double t_uncertainty_parameter) {

    const unsigned int n_facilities = t_instance.n_facilities();
    const unsigned int n_customers = t_instance.n_customers();

    Model result;

    auto stage = result.add_user_attr<unsigned int>(0);

    /// VARIABLES

    // First-stage variables
    auto x = result.add_vars(Dim<1>(n_facilities), 0., 1., Binary, 0., "x");
    auto q = result.add_vars(Dim<1>(n_facilities), 0., 1., Continuous, 0., "q");

    // Second-stage variables
    auto y = result.add_vars(Dim<2>(n_facilities, n_customers), 0., 1., Continuous, 0., "y");
    auto z = result.add_vars(Dim<2>(n_facilities, n_customers), 0., 1., Binary, 0., "z");
    auto v = result.add_vars(Dim<1>(n_facilities), 0., Inf, Continuous, 0., "v");
    auto r = result.add_vars(Dim<1>(n_facilities), 0., Inf, Continuous, 0., "r");

    // Adversary dual variables
    auto lambda = result.add_var(0., Inf, Continuous, 0., "lambda");
    auto pi = result.add_vars(Dim<2>(n_facilities, n_customers), 0., Inf, Continuous, 0., "pi");

    /// CONSTRAINTS

    // Facility activation constraints
    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        result.add_ctr(q[i] <= x[i]);
    }

    // Capacity constraints
    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        auto c = result.add_ctr(idol_Sum(j, Range(n_customers), t_instance.demand(j) * y[i][j]) <= t_instance.max_capacity(i) * q[i]);
        result.set<unsigned int>(stage, c, 1);
    }

    // Demand constraints
    for (unsigned int j = 0 ; j < n_customers ; ++j) {
        auto c = result.add_ctr(idol_Sum(i, Range(n_facilities), y[i][j]) >= 1);
        result.set<unsigned int>(stage, c, 1);
    }

    // Arc activation constraints
    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        for (unsigned int j = 0 ; j < n_customers ; ++j) {
            auto c = result.add_ctr(y[i][j] <= z[i][j]);
            result.set<unsigned int>(stage, c, 1);
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

    return std::make_tuple(std::move(result), x, q, stage);
}

Vector<Var, 1> get_reformulated(const Model& t_original_model, const Vector<Var, 1>& t_vec, const UserAttr& t_attr) {
    Vector<Var, 1> result;
    result.reserve(t_vec.size());

    for (const auto& var : t_vec) {
        result.emplace_back( t_original_model.get<Var>(t_attr, var) );
    }

    return result;
}

int main(int t_argc, const char** t_argv) {

    auto instance = read_instance("/home/henri/CLionProjects/AB_AdjustableRobustOptimizationWithObjectiveUncertainty/FLP/data/instance_4_8_110__2.txt");

    const double uncertainty_parameter = 1;

    auto [single_stage, x, q, second_stage_flag] = make_single_stage_model(instance, Convex, Polyhedral, uncertainty_parameter);
    Reformulations::DantzigWolfe result(single_stage, second_stage_flag);

    auto integer_branching_candidates = get_reformulated(single_stage, x, result.reformulated_variable());
    auto continuous_branching_candidates = get_reformulated(single_stage, q, result.reformulated_variable());

    // Making continuous relaxation
    for (const auto& var : integer_branching_candidates) {
        result.restricted_master_problem().set(Attr::Var::Type, var, Continuous);
    }

    auto solver = make_solver(
            result.restricted_master_problem(),
            result.alpha(1),
            result.subproblem(1),
            integer_branching_candidates,
            continuous_branching_candidates
        );

    Solvers::Gurobi ub(single_stage);
    ub.solve();

    solver.set(Param::Algorithm::BestObjStop, ub.primal_solution().objective_value());

    solver.add_callback<MostActiveHeuristic>(integer_branching_candidates, continuous_branching_candidates);

    solver.solve();

    std::cout << "Time: " << (ub.time().count() + solver.time().count()) << " s" << std::endl;

    return 0;
}
