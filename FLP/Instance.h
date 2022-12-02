//
// Created by henri on 30/11/22.
//

#ifndef AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_INSTANCE_H
#define AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_INSTANCE_H

#include <vector>
#include <string>

class Instance {
    // For sites
    std::vector<double> m_capacity_fixed_costs;
    std::vector<double> m_per_unit_capacity_costs;
    std::vector<double> m_max_capacities;

    // For clients
    std::vector<double> m_demands;

    // For connections
    std::vector<std::vector<double>> m_transportation_fixed_cost_nominals;
    std::vector<std::vector<double>> m_transportation_fixed_cost_deviations;
    std::vector<std::vector<double>> m_per_unit_transportation_cost_nominals;
    std::vector<std::vector<double>> m_per_unit_transportation_cost_deviations;

public:
    Instance(unsigned int t_n_facilities, unsigned int t_n_customers);

    [[nodiscard]] unsigned int n_facilities() const { return m_capacity_fixed_costs.size(); }
    [[nodiscard]] unsigned int n_customers() const { return m_demands.size(); }

    [[nodiscard]] double capacity_fixed_cost(unsigned int t_i) const { return m_capacity_fixed_costs[t_i]; }
    [[nodiscard]] double per_unit_capacity_cost(unsigned int t_i) const { return m_per_unit_capacity_costs[t_i]; }
    [[nodiscard]] double max_capacity(unsigned int t_i) const { return m_max_capacities[t_i]; }
    [[nodiscard]] double demand(unsigned int t_j) const { return m_demands[t_j]; }
    [[nodiscard]] double transportation_fixed_cost_nominal(unsigned int t_i, unsigned int t_j) const { return m_transportation_fixed_cost_nominals[t_i][t_j]; }
    [[nodiscard]] double transportation_fixed_cost_deviation(unsigned int t_i, unsigned int t_j) const { return m_transportation_fixed_cost_deviations[t_i][t_j]; }
    [[nodiscard]] double per_unit_transportation_cost_nominal(unsigned int t_i, unsigned int t_j) const { return m_per_unit_transportation_cost_nominals[t_i][t_j]; }
    [[nodiscard]] double per_unit_transportation_cost_deviation(unsigned int t_i, unsigned int t_j) const { return m_per_unit_transportation_cost_deviations[t_i][t_j]; }

    void set_capacity_fixed_cost(unsigned int t_i, double t_value) { m_capacity_fixed_costs[t_i] = t_value; }
    void set_per_unit_capacity_cost(unsigned int t_i, double t_value) { m_per_unit_capacity_costs[t_i] = t_value; }
    void set_max_capacity(unsigned int t_i, double t_value) { m_max_capacities[t_i] = t_value; }
    void set_demand(unsigned int t_j, double t_value) { m_demands[t_j] = t_value; }
    void set_transportation_fixed_cost_nominal(unsigned int t_i, unsigned int t_j, double t_value) { m_transportation_fixed_cost_nominals[t_i][t_j] = t_value; }
    void set_transportation_fixed_cost_deviation(unsigned int t_i, unsigned int t_j, double t_value) { m_transportation_fixed_cost_deviations[t_i][t_j] = t_value; }
    void set_per_unit_transportation_cost_nominal(unsigned int t_i, unsigned int t_j, double t_value) { m_per_unit_transportation_cost_nominals[t_i][t_j] = t_value; }
    void set_per_unit_transportation_cost_deviation(unsigned int t_i, unsigned int t_j, double t_value) { m_per_unit_transportation_cost_deviations[t_i][t_j] = t_value; }
};

Instance read_instance(const std::string& t_path_to_file);

#endif //AB_ADJUSTABLEROBUSTOPTIMIZATIONWITHOBJECTIVEUNCERTAINTY_INSTANCE_H
