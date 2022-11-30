cmake_minimum_required(VERSION 3.23)
project(AB_AdjustableRobustOptimizationWithObjectiveUncertainty)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
        idol
        GIT_REPOSITORY https://github.com/hlefebvr/idol.git
        GIT_TAG        v0.0.2-alpha
)

FetchContent_MakeAvailable(idol)

add_subdirectory(FLP)