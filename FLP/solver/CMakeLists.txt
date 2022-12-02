add_executable(FLP FLP_main.cpp ../Instance.h ../Instance.cpp MostInfeasibleWithSpatialBranching.h make_solver.h NodeBaP.h)
target_link_libraries(FLP PUBLIC idol)