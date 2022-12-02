#include <iostream>
#include "../Instance.h"
#include "make_solver.h"
#include <modeling.h>

enum UncertaintySet { Polyhedral, Ellipsoidal };
enum ObjectiveType { Convex, Linearized };

std::tuple<Model, Vector<Var>, Vector<Var>, Vector<Var, 2>, Vector<Var, 2>>
create_subproblem(const Instance& t_instance, ObjectiveType t_objective_type) {

    const unsigned int n_facilities = t_instance.n_facilities();
    const unsigned int n_customers = t_instance.n_customers();

    Model result;

    auto q = result.add_vars(Dim<1>(n_facilities), 0., 1., Continuous, 0., "q");
    auto y = result.add_vars(Dim<2>(n_facilities, n_customers), 0., 1., Continuous, 0., "y");
    auto z = result.add_vars(Dim<2>(n_facilities, n_customers), 0., 1., Binary, 0., "z");
    auto v = result.add_vars(Dim<1>(n_facilities), 0., Inf, Continuous, 0., "v");
    auto r = result.add_vars(Dim<1>(n_facilities), 0., Inf, Continuous, 0., "r");

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        result.add_ctr(idol_Sum(j, Range(n_customers), t_instance.demand(j) * y[i][j]) <= t_instance.max_capacity(i) * q[i]);
    }

    for (unsigned int j = 0 ; j < n_customers ; ++j) {
        result.add_ctr(idol_Sum(i, Range(n_facilities), y[i][j]) >= 1);
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

    return std::make_tuple(std::move(result), q, r, y, z);
}

std::tuple<Model, Var, Vector<Var>> create_rmp(const Instance& t_instance,
                                  const Vector<Var>& t_q,
                                  const Vector<Var>& t_r,
                                  const Vector<Var, 2>& t_y,
                                  const Vector<Var, 2>& t_z,
                                  UncertaintySet t_uncertainty_set,
                                  double t_uncertainty_parameter) {

    const unsigned int n_facilities = t_instance.n_facilities();
    const unsigned int n_customers = t_instance.n_customers();

    Model result;

    auto x = result.add_vars(Dim<1>(n_facilities), 0., 1., Continuous, 0., "x");
    auto alpha = result.add_var(0., 1., Continuous, 0, "alpha");
    auto lambda = result.add_var(0., Inf, Continuous, 0., "lambda");
    auto pi = result.add_vars(Dim<2>(n_facilities, n_customers), 0., Inf, Continuous, 0., "pi");

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        result.add_ctr(!t_q[i] * alpha <= x[i]);
    }

    result.add_ctr(alpha == 1);

    if (t_uncertainty_set == Polyhedral) {
        std::cout << "WARNING COSTS ARE WRONGLY COMPUTED" << std::endl;

        for (unsigned int i = 0; i < n_facilities; ++i) {
            for (unsigned int j = 0; j < n_customers; ++j) {
                result.add_ctr(lambda + pi[i][j] >=
                               (t_instance.transportation_fixed_cost_deviation(i ,j) * !t_z[i][j]
                                + t_instance.per_unit_transportation_cost_deviation(i, j) * !t_y[i][j]) * alpha);
            }
        }
    } else {
        std::cout << "WARNING COSTS ARE WRONGLY COMPUTED" << std::endl;
        throw std::runtime_error("Not implemented.");
    }

    Expr objective = t_uncertainty_parameter * lambda
                    + idol_Sum(i, Range(n_facilities), idol_Sum(j, Range(n_customers), pi[i][j]))
                    + idol_Sum(i, Range(n_facilities), t_instance.capacity_fixed_cost(i) * x[i])
                    + idol_Sum(i, Range(n_facilities), t_instance.per_unit_capacity_cost(i) * t_instance.max_capacity(i) * !t_q[i] * alpha)
                    + idol_Sum(i, Range(n_facilities), !t_r[i] * alpha)
                    + idol_Sum(i, Range(n_facilities),
                       idol_Sum(j, Range(n_customers),
                                (
                                        t_instance.transportation_fixed_cost_nominal(i, j) * !t_z[i][j]
                                        + t_instance.per_unit_transportation_cost_nominal(i, j) * !t_y[i][j]
                                ) * alpha
                        ))
                    ;

    result.set(Attr::Obj::Expr, std::move(objective));

    return std::make_tuple(std::move(result), alpha, x);
}

int main(int t_argc, const char** t_argv) {

    auto instance = read_instance("/home/henri/CLionProjects/AB_AdjustableRobustOptimizationWithObjectiveUncertainty/FLP/data/instance_4_8_110__2.txt");

    const double uncertainty_parameter = 1;

    auto [sp, q, r, y, z] = create_subproblem(instance, Convex);
    auto [rmp, alpha, x] = create_rmp(instance, q, r, y, z, Polyhedral, uncertainty_parameter);

    Log::set_level(Info);

    auto solver = make_solver(rmp, alpha, sp, x, q);
    solver.solve();

    std::cout << solver.primal_solution().objective_value() << std::endl;
    std::cout << solver.time().count() << std::endl;

    return 0;
}
