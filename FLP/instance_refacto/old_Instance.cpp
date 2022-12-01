//
// Created by henri on 11/03/22.
//

#include <random>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "old_Instance.h"


class old::Instance::Generator {
    Instance& m_instance;
    std::random_device m_rd;
    std::mt19937 m_engine;

    std::vector<std::pair<double, double>> m_sites;
    std::vector<std::pair<double, double>> m_clients;

    void generate_q();
    void generate_f();
    void generate_u();
    void generate_n_points(unsigned int t_n, std::vector<std::pair<double, double>>& t_vec);
    double distance(const std::pair<double, double>& t_a, const std::pair<double, double>& t_b);
    void compute_t();
    void generate_d(double t_capacity_demand_ratio);
    void generate_C();
    void generate_R();
    void generate_alpha();
    void generate_beta();
    void generate_gamma();
public:
    Generator(Instance& t_instance, unsigned int t_n_sites, unsigned int t_n_clients, double capacity_demand_ratio);
};

old::Instance::Generator::Generator(Instance &t_instance,
                               unsigned int t_n_sites,
                               unsigned int t_n_clients,
                               double capacity_demand_ratio)
                               : m_instance(t_instance), m_engine(m_rd()) {

    m_instance.m_n_sites = t_n_sites;
    m_instance.m_n_clients = t_n_clients;

    generate_q();
    generate_f();
    generate_u();
    generate_n_points(t_n_sites, m_sites);
    generate_n_points(t_n_clients, m_clients);
    compute_t();
    generate_d(capacity_demand_ratio);
    generate_C();
    generate_R();
    generate_alpha();
    generate_beta();
    generate_gamma();
}

void old::Instance::Generator::generate_q() {

    std::uniform_real_distribution<double> dist(10., 160.);

    m_instance.m_q.reserve(m_instance.m_n_sites);
    for (unsigned int i = 0 ; i < m_instance.m_n_sites ; ++i) {
        m_instance.m_q.emplace_back(dist(m_engine));
    }

}

void old::Instance::Generator::generate_f() {

    std::uniform_real_distribution<double> dist1(0., 90.);
    std::uniform_real_distribution<double> dist2(100., 110.);

    m_instance.m_f.reserve(m_instance.m_n_sites);
    m_instance.m_u.reserve(m_instance.m_n_sites);
    for (unsigned int i = 0 ; i < m_instance.m_n_sites ; ++i) {
        m_instance.m_f.emplace_back(dist1(m_engine));
        m_instance.m_u.emplace_back(dist2(m_engine) / std::sqrt(m_instance.m_q[i]));
        if (m_instance.m_n_sites * m_instance.m_n_clients <= 500) {
            m_instance.m_f.back() *= 2.;
        }
    }

}

void old::Instance::Generator::generate_u() {
    // done in Generator::generate_f
}

void old::Instance::Generator::generate_n_points(unsigned int t_n, std::vector<std::pair<double, double>> &t_vec) {

    std::uniform_real_distribution<double> dist(0., 1.);

    t_vec.reserve(t_n);
    for (unsigned int i = 0 ; i < t_n ; ++i) {
        t_vec.emplace_back(std::make_pair(dist(m_engine), dist(m_engine)));
    }

}

double old::Instance::Generator::distance(const std::pair<double, double> &t_a, const std::pair<double, double> &t_b) {
    return std::sqrt( std::pow(t_a.first - t_b.first, 2) + std::pow(t_a.second - t_b.second, 2) );
}

void old::Instance::Generator::compute_t() {

    const double deviation = .5;

    m_instance.m_t_bar.resize(m_instance.m_n_sites);
    m_instance.m_t_tilde.resize(m_instance.m_n_sites);
    for (unsigned int i = 0 ; i < m_instance.m_n_sites ; ++i) {
        m_instance.m_t_bar[i].reserve(m_instance.m_n_clients);
        m_instance.m_t_tilde[i].reserve(m_instance.m_n_clients);
        for (unsigned int j = 0 ; j < m_instance.m_n_clients ; ++j) {
            const double d_ij = distance(m_sites[i], m_clients[j]);
            const double t_bar = 10 * d_ij;
            const double t_tilde = t_bar * deviation;
            m_instance.m_t_bar[i].emplace_back(t_bar);
            m_instance.m_t_tilde[i].emplace_back(t_tilde);
        }
    }

}

void old::Instance::Generator::generate_C() {

    const double sum_demands = std::accumulate(m_instance.m_d.begin(), m_instance.m_d.end(), 0.);
    const double avg_demand = sum_demands / m_instance.m_n_clients;

    //m_instance.m_C = std::ceil(avg_demand / 2.);

    auto it = std::max_element(m_instance.m_d.begin(), m_instance.m_d.end());
    m_instance.m_C = std::ceil(*it);

}

void old::Instance::Generator::generate_R() {

    std::uniform_real_distribution<double> dist(50., 100.);

    m_instance.m_R.resize(m_instance.m_n_sites);
    for (unsigned int i = 0 ; i < m_instance.m_n_sites ; ++i) {
        m_instance.m_R[i].reserve(m_instance.m_n_clients);
        for (unsigned int j = 0 ; j < m_instance.m_n_clients ; ++j) {
            const double d_ij = distance(m_sites[i], m_clients[j]);
            m_instance.m_R[i].emplace_back(dist(m_engine) * d_ij);
        }
    }

}

void old::Instance::Generator::generate_alpha() {

    m_instance.m_alpha.reserve(m_instance.m_n_sites);
    for (unsigned int i = 0 ; i < m_instance.m_n_sites ; ++i) {
        m_instance.m_alpha.emplace_back(.75);
    }

}

void old::Instance::Generator::generate_beta() {

    m_instance.m_beta.reserve(m_instance.m_n_sites);
    for (unsigned int i = 0 ; i < m_instance.m_n_sites ; ++i) {
        m_instance.m_beta.emplace_back(.75);
    }

}

void old::Instance::Generator::generate_gamma() {

    m_instance.m_gamma.reserve(m_instance.m_n_sites);
    for (unsigned int i = 0 ; i < m_instance.m_n_sites ; ++i) {
        m_instance.m_gamma.emplace_back(1.);
    }

}

void old::Instance::Generator::generate_d(double t_capacity_demand_ratio) {

    m_instance.m_d.reserve(m_instance.m_n_clients);

    // generate random U(0,1) for weights repartition of demands
    double sum_d_j = 0;
    for (unsigned int i = 0 ; i < m_instance.m_n_clients ; ++i) {
        m_instance.m_d.emplace_back(std::uniform_real_distribution<double>(0, 1)(m_engine));
        sum_d_j += m_instance.m_d.back();
    }

    // scale
    double sum_q_i = std::accumulate(m_instance.m_q.begin(), m_instance.m_q.end(), 0.);
    const double scaling_factor = sum_q_i / (t_capacity_demand_ratio * sum_d_j);
    std::for_each(m_instance.m_d.begin(), m_instance.m_d.end(), [scaling_factor](double& d_j){ d_j *= scaling_factor; });

}

old::Instance::Instance(unsigned int t_n_sites, unsigned int t_n_clients, double t_capacity_demand_ratio) {
    Generator generator(*this, t_n_sites, t_n_clients, t_capacity_demand_ratio);
}

std::ostream& operator<<(std::ostream& t_os, const std::vector<double>& t_vec) {
    std::for_each(t_vec.begin(), t_vec.end(), [&t_os](double elem){ t_os << elem << '\t'; });
    return t_os << '\n';
}

std::ostream& operator<<(std::ostream& t_os, const std::vector<std::vector<double>>& t_vec) {
    std::for_each(t_vec.begin(), t_vec.end(), [&t_os](const std::vector<double>& elem){ t_os << elem; });
    return t_os;
}

std::ostream& operator<<(std::ostream& t_os, const old::Instance& t_instance) {

    auto user_precision = t_os.precision();

    t_os << std::setprecision(2) << std::fixed;
    t_os << t_instance.m_n_sites << '\t' << t_instance.m_n_clients << '\n';
    t_os << t_instance.m_f;
    t_os << t_instance.m_u;
    t_os << t_instance.m_q;
    t_os << t_instance.m_d;
    t_os << t_instance.m_t_bar;
    t_os << t_instance.m_t_tilde;
    t_os << t_instance.m_R;
    t_os << std::setprecision(user_precision);

    return t_os;
}

old::Instance::Instance(const std::string &t_path) {

    std::ifstream file(t_path);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open instance file " + t_path);
    }

    file >> m_n_sites >> m_n_clients;
    read(file, m_f, m_n_sites);
    read(file, m_d, m_n_clients);
    read(file, m_t_bar, m_n_sites, m_n_clients);
    read(file, m_q, m_n_sites);
    file >> m_C;
    read(file, m_R, m_n_sites, m_n_clients);
    read(file, m_t_tilde, m_n_sites, m_n_clients);
    read(file, m_alpha, m_n_sites);
    read(file, m_beta, m_n_sites);
    read(file, m_gamma, m_n_sites);
    read(file, m_u, m_n_sites);

    file.close();

}

void old::Instance::read(std::ifstream &t_file, std::vector<double> &t_dest, unsigned int t_n) {

    t_dest.resize(t_n);
    for (unsigned int i = 0 ; i < t_n ; ++i) {
        t_file >> t_dest[i];
    }

}

void old::Instance::read(std::ifstream &t_file, std::vector<std::vector<double>> &t_dest, unsigned int t_m, unsigned int t_n) {

    t_dest.resize(t_m);
    for (unsigned int i = 0 ; i < t_m ; ++i) {
        read(t_file, t_dest[i], t_n);
    }

}

double old::Instance::capacity_demand_ratio() const {
    return std::accumulate(m_q.begin(), m_q.end(), 0.) / std::accumulate(m_d.begin(), m_d.end(), 0.);
}

void old::Instance::compute_K() {

    double max_demand = *std::max_element(m_d.begin(), m_d.end());

    m_K = std::ceil(max_demand / m_C);

}

double old::Instance::q_max() const {
    return *std::max_element(m_q.begin(), m_q.end());
}
