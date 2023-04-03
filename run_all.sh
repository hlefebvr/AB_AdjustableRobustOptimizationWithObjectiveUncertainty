EXECUTABLE=./cmake-build-debug/FLP/FLP
INSTANCES=./FLP/data/

for FILE in $INSTANCES/*
do
  echo "# At $(date), running instance ${FILE}." >> history.txt
  $EXECUTABLE $FILE >> logs.txt
done
