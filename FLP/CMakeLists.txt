
add_executable(FLP FLP_main.cpp Instance.h Instance.cpp ../../solver/MostInfeasibleWithSpatialBranching.h AdjustableFLP.h AdjustableFLP.cpp ../solver/tolerances.h ../solver/MostActiveHeuristic.h ../solver/MostActiveHeuristic.cpp ../solver/NodeWithActiveColumns.cpp)
target_link_libraries(FLP PUBLIC idol)
