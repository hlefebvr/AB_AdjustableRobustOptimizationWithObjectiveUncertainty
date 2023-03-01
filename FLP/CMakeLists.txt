
add_executable(FLP FLP_main.cpp Instance.h Instance.cpp ../../solver/MostInfeasibleWithSpatialBranching.h ../../solver/NodeBaP.h AdjustableFLP.h AdjustableFLP.cpp ../../solver/Solver.h ../../solver/MostActiveHeuristic.h)
target_link_libraries(FLP PUBLIC idol)

add_subdirectory(instance_refacto)