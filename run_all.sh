EXECUTABLE=./cmake-build-debug/FLP/FLP
INSTANCES=./FLP/data/

echo "result,instance,uncertainty_type,uncertainty_param,objective_type,static_ub,objective,status,reason,n_created_nodes,n_solved_nodes,n_columns,rel_gap,static_adjust_gap,total_time,rmp_time,pricing_time"

for FILE in $INSTANCES/*
do
  echo "# At $(date), running instance ${FILE}." >> history.txt
  $EXECUTABLE $FILE >> logs.txt
done
