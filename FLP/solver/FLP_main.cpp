#include <iostream>
#include "../Instance.h"
#include <modeling.h>
#include <algorithms.h>

enum UncertaintySet { Polyhedral, Ellipsoidal };
enum ObjectiveType { Convex, Linearized };

std::tuple<Model, Vector<Var>, Vector<Var, 2>, Vector<Var, 2>>
create_subproblem(const Instance& t_instance, ObjectiveType t_objective_type) {

    const unsigned int n_facilities = t_instance.n_facilities();
    const unsigned int n_customers = t_instance.n_customers();

    Model result;

    auto q = result.add_vars(Dim<1>(n_facilities), 0., 1., Continuous, 0., "q");
    auto y = result.add_vars(Dim<2>(n_facilities, n_customers), 0., 1., Continuous, 0., "y");
    auto z = result.add_vars(Dim<2>(n_facilities, n_customers), 0., 1., Binary, 0., "z");
    auto v = result.add_vars(Dim<1>(n_facilities), 0., 1., Continuous, 0., "v");
    auto r = result.add_vars(Dim<1>(n_facilities), 0., Inf, Continuous, 0., "r");

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        result.add_ctr(idol_Sum(j, Range(n_customers), t_instance.demand(j) * y[i][j]) <= q[i]);
    }

    for (unsigned int j = 0 ; j < n_customers ; ++j) {
        result.add_ctr(idol_Sum(i, Range(n_facilities), y[i][j]) == 1);
    }

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        for (unsigned int j = 0 ; j < n_customers ; ++j) {
            result.add_ctr(y[i][j] <= z[i][j]);
        }
    }

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        result.add_ctr(v[i] == idol_Sum(j, Range(n_customers), t_instance.demand(j) * y[i][j]));
    }

    if (t_objective_type == Convex) {
        for (unsigned int i = 0; i < n_facilities; ++i) {
            result.add_ctr(r[i] >= .75 * v[i] + .75 * v[i] * v[i]);
        }
    } else {
        throw std::runtime_error("Not implemented.");
    }

    return std::make_tuple(std::move(result), q, y, z);
}

std::tuple<Model, Var> create_rmp(const Instance& t_instance,
                                  const Vector<Var>& t_q,
                                  const Vector<Var, 2>& t_y,
                                  const Vector<Var, 2>& t_z,
                                  UncertaintySet t_uncertainty_set) {

    const unsigned int n_facilities = t_instance.n_facilities();
    const unsigned int n_customers = t_instance.n_customers();

    Model result;

    auto x = result.add_vars(Dim<1>(n_facilities), 0., 1., Binary, 0., "x");
    auto alpha = result.add_var(0., 1., Continuous, 0, "alpha");

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        result.add_ctr(!t_q[i] * alpha <= t_instance.max_capacity(i) * x[i]);
    }

    result.add_ctr(alpha == 1);

    if (t_uncertainty_set == Polyhedral) {
        auto lambda = result.add_var(0., Inf, Continuous, 0., "lambda");
        auto pi = result.add_vars(Dim<2>(n_facilities, n_customers), 0., Inf, Continuous, 0., "pi");

        for (unsigned int i = 0; i < n_facilities; ++i) {
            for (unsigned int j = 0; j < n_customers; ++j) {
                result.add_ctr(lambda + pi[i][j] >= 0 * t_z[i][j] + 0 * t_y[i][j]); // TODO
            }
        }
    } else {
        throw std::runtime_error("Not implemented.");
    }


    return std::make_tuple(std::move(result), alpha);
}

int main() {

    auto instance = read_instance("/home/henri/CLionProjects/AB_AdjustableRobustOptimizationWithObjectiveUncertainty/FLP/data/racflp_s4_c8_r150_sl10_su100_0.txt");

    auto [sp, q, y, z] = create_subproblem(instance, Convex);
    auto [rmp, alpha] = create_rmp(instance, q, y, z, Polyhedral);

    return 0;
}
