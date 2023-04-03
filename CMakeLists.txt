cmake_minimum_required(VERSION 3.22)
project(AB_AdjustableRobustOptimizationWithObjectiveUncertainty)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

set(USE_GUROBI ON)
set(USE_MOSEK ON)
set(USE_EIGEN ON)

FetchContent_Declare(
        idol
        GIT_REPOSITORY https://github.com/hlefebvr/idol.git
        GIT_TAG        v0.2.1-alpha
)

FetchContent_MakeAvailable(idol)

add_subdirectory(FLP)