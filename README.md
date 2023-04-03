# Adjustable robust optimization with objective uncertainty

![GitHub license](https://img.shields.io/github/license/hlefebvr/AB_AdjustableRobustOptimizationWithObjectiveUncertainty)
![Maintained](https://img.shields.io/maintenance/yes/2023)
![GitHub issues](https://img.shields.io/github/issues-raw/hlefebvr/AB_AdjustableRobustOptimizationWithObjectiveUncertainty)

This repository contains an implementation of the method introduced in [1] for solving adjustable 
robust optimization problems with objective uncertainty. The instances used in the computational
section are also available.

## Dependencies

The implementation of the branch-and-price algorithm with spatial branching is done in C++17 and
uses the [Idol](https://github.com/hlefebvr/idol) C++ library.

For CMake users (see instructions bellow), the library will automatically be downloaded and 
locally installed as a cmake dependence. 

Note that [Idol](https://github.com/hlefebvr/idol) itself may have other dependencies depending on what you want to achieve.
For instance, you probably want to use an external solver (e.g., Gurobi, Mosek or GLPK).
To do so, you should then specify the CMake option `-DUSE_GUROBI=YES`, `-DUSE_MOSEK=YES` or `-DUSE_GLPK=YES`
accordingly. 

**About using Mosek with quadratic constraints**: The interface of [Idol](https://github.com/hlefebvr/idol)
is based on functional expressions, e.g., like $$\sum_{j=1}^n a_{ij}x_j + \sum_{j=1}^n\sum_{k=1}^n q_{jk}^ix_jx_k \le b_i.$$
The C++ Mosek interface, instead, is based on conic expressions, e.g., like $(x_0, \textbf{Fx}) \in \mathcal Q^n$. 
To make the conversion between the Mosek interface and
the Idol interface, one needs to compute an eigen value decomposition. 
This is automatically done by Idol using the [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page) C++ library.
Thus, if you intend to use Idol to interface with Mosek and to use quadratic expressions, you need to set the `-DUSE_EIGEN=YES` cmake
option as well (Note that Eigen is a header-only library and that no installation is needed).

**About using Gurobi with quadratic constraints**: To the best of our knowledge, Gurobi does not return a Farkas certificate 
for infeasible SOCPs. Thus, one should turn off Farkas pricing when dealing with infeasible restricted master problems if this one
contains SOCP constraints. In this case, Idol will introduce artificial variables (similar to Phase I Simplex) to handle
the infeasible cases. The value for these artificial columns can be controlled by the `with_artificial_variables_cost`
method (default value: 10+9). Note that this is likely to lead to numerical instabilities.

## Build and run (CMake)

- Create a `build/` folder at the root of the repository;
- Change directory to `build/`;
- Run CMake (see options bellow);
```
cmake -DUSE_GUROBI=YES -DUSE_MOSEK=YES -DUSE_EIGEN=YES ..
```
- Run make;
```
make
```
- Run the executable.
```
./FLP/FLP FLP/data/instance_4_8_110__0.txt
```

**N.b.:** To help CMake find Gurobi, Mosek and Eigen. Please have your environment variables `GUROBI_HOME`, `MOSEK_HOME` 
and `EIGEN_HOME` properly configured.

*****

[1] *Adjustable robust optimization with objective uncertainty*, Detienne B., Lefebvre H., Malaguti E.,
Monaci M. (2023).
