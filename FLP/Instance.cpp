//
// Created by henri on 30/11/22.
//
#include <fstream>
#include "Instance.h"

Instance::Instance(unsigned int t_n_facilities, unsigned int t_n_customers)
    : m_capacity_fixed_costs(t_n_facilities),
      m_per_unit_capacity_costs(t_n_facilities),
      m_max_capacities(t_n_facilities),
      m_demands(t_n_customers),
      m_per_unit_transportation_cost_nominals(t_n_facilities, std::vector<double>(t_n_customers)),
      m_per_unit_transportation_cost_deviations(t_n_facilities, std::vector<double>(t_n_customers)),
      m_transportation_fixed_cost_nominals(t_n_facilities, std::vector<double>(t_n_customers)),
      m_transportation_fixed_cost_deviations(t_n_facilities, std::vector<double>(t_n_customers))
{

}

Instance read_instance(const std::string& t_path_to_file) {

    std::ifstream file(t_path_to_file);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open instance file.");
    }

    unsigned int n_facilities, n_customers;
    file >> n_facilities >> n_customers;

    Instance result(n_facilities, n_customers);

    double placeholder;

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        file >> placeholder;
        result.set_per_unit_capacity_cost(i, placeholder);
    }

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        file >> placeholder;
        result.set_capacity_fixed_cost(i, placeholder);
    }

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        file >> placeholder;
        result.set_max_capacity(i, placeholder);
    }

    for (unsigned int j = 0 ; j < n_customers ; ++j) {
        file >> placeholder;
        result.set_demand(j, placeholder);
    }

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        for (unsigned int j = 0 ; j < n_customers ; ++j) {
            file >> placeholder;
            result.set_per_unit_transportation_cost_nominal(i, j, placeholder);
        }
    }

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        for (unsigned int j = 0 ; j < n_customers ; ++j) {
            file >> placeholder;
            result.set_per_unit_transportation_cost_deviation(i, j, placeholder);
        }
    }

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        for (unsigned int j = 0 ; j < n_customers ; ++j) {
            file >> placeholder;
            result.set_transportation_fixed_cost_nominal(i, j, placeholder);
        }
    }

    for (unsigned int i = 0 ; i < n_facilities ; ++i) {
        for (unsigned int j = 0 ; j < n_customers ; ++j) {
            file >> placeholder;
            result.set_transportation_fixed_cost_deviation(i, j, placeholder);
        }
    }

    return result;

}

