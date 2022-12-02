cmake_minimum_required(VERSION 3.23)
project(AB_AdjustableRobustOptimizationWithObjectiveUncertainty)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

set(USE_GUROBI ON)

FetchContent_Declare(
        idol
        GIT_REPOSITORY https://github.com/hlefebvr/idol.git
        GIT_TAG        origin/dev # v0.0.2-alpha
)

FetchContent_MakeAvailable(idol)

add_subdirectory(FLP)