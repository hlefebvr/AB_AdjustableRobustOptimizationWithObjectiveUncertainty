EXECUTABLE=/home/henri/CLionProjects/AB_AdjustableRobustOptimizationWithObjectiveUncertainty/cmake-build-debug/FLP/instance_refacto/instance_refacto

for FILE in /home/henri/CLionProjects/AB_AdjustableRobustOptimizationWithObjectiveUncertainty/FLP/instance_refacto/old_data/*
do
  $EXECUTABLE $FILE
done