//
// Created by henri on 11/03/22.
//

#ifndef ARCCFLP_INSTANCE_H
#define ARCCFLP_INSTANCE_H

#include <vector>
#include <cassert>

namespace old {
    class Instance;
}

std::ostream& operator<<(std::ostream& t_os, const std::vector<double>& t_vec);
std::ostream& operator<<(std::ostream& t_os, const std::vector<std::vector<double>>& t_vec);
std::ostream& operator<<(std::ostream& t_os, const old::Instance& t_instance);

class old::Instance {

    unsigned int m_n_sites {0};
    unsigned int m_n_clients {0};

    // Sites
    std::vector<double> m_f;
    std::vector<double> m_u;
    std::vector<double> m_q;
    std::vector<double> m_alpha;
    std::vector<double> m_beta;
    std::vector<double> m_gamma;

    // Clients
    std::vector<double> m_d;

    // Connections
    std::vector<std::vector<double>> m_t_bar;
    std::vector<std::vector<double>> m_t_tilde;
    std::vector<std::vector<double>> m_R;

    // Containers
    double m_C {0};
    unsigned int m_K {0}; // max number of containers needed on every arc

    class Generator;
    friend std::ostream& ::operator<<(std::ostream& t_os, const Instance& t_instance);

    void read(std::ifstream& t_file, std::vector<double>& t_dest, unsigned int t_n);
    void read(std::ifstream& t_file, std::vector<std::vector<double>>& t_dest, unsigned int t_m, unsigned int t_n);
    void compute_K();
public:
    Instance(unsigned int t_n_sites, unsigned int t_n_clients, double t_capacity_demand_ratio);
    explicit Instance(const std::string& t_path);

    [[nodiscard]] unsigned int n_sites() const { return m_n_sites; }
    [[nodiscard]] unsigned int n_clients() const { return m_n_clients; }
    [[nodiscard]] unsigned int K() const { if (m_K == 0) { const_cast<Instance *>(this)->compute_K(); } return m_K; }
    [[nodiscard]] double f(unsigned int t_i) const { return m_f[t_i]; }
    [[nodiscard]] double u(unsigned int t_i) const { return m_u[t_i]; }
    [[nodiscard]] double q(unsigned int t_i) const { return m_q[t_i]; }
    [[nodiscard]] double alpha(unsigned int t_i) const { return m_alpha[t_i]; }
    [[nodiscard]] double beta(unsigned int t_i) const { return m_beta[t_i]; }
    [[nodiscard]] double gamma(unsigned int t_i) const { assert(m_gamma[t_i] == 1.); return m_gamma[t_i]; }
    [[nodiscard]] double d(unsigned int t_j) const { return m_d[t_j]; }
    [[nodiscard]] double t_bar(unsigned int t_i, unsigned int t_j) const { return m_t_bar[t_i][t_j]; }
    [[nodiscard]] double t_tilde(unsigned int t_i, unsigned int t_j) const { return m_t_tilde[t_i][t_j]; }
    [[nodiscard]] double R(unsigned int t_i, unsigned int t_j) const { return m_R[t_i][t_j]; }
    [[nodiscard]] double C() const { return m_C; }
    [[nodiscard]] double capacity_demand_ratio() const;
    [[nodiscard]] double q_max() const;
};


#endif //ARCCFLP_INSTANCE_H
