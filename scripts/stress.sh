CMD_PATH="./build/similarity"
STRESS_DIR="stress"
REPORT_FILE="$STRESS_DIR/report.txt"
INPUT_FILE="$STRESS_DIR/input.txt"
OUTPUT_FILE_ST="$STRESS_DIR/output_st.txt"
OUTPUT_FILE_DYN="$STRESS_DIR/output_dyn.txt"
DIFFS_FILE="$STRESS_DIR/diffs.txt"
TIMES=5

rm -rf $STRESS_DIR
mkdir $STRESS_DIR
touch $REPORT_FILE
touch $INPUT_FILE
echo "4 4 4 4 4" > $INPUT_FILE

# Стресс-тест последовательной версии
bash ./scripts/build.sh

echo $TIMES >> $REPORT_FILE
for ((i=0; i<TIMES; i++))
do
    /usr/bin/time -a -f"%e" -o $REPORT_FILE $CMD_PATH < $INPUT_FILE >> $OUTPUT_FILE_ST
done

# Стресс-тест параллельной версии
bash ./scripts/build.sh -p

echo $TIMES >> $REPORT_FILE
for ((i=0; i<TIMES; i++))
do
    /usr/bin/time -a -f"%e" -o $REPORT_FILE $CMD_PATH < $INPUT_FILE >> $OUTPUT_FILE_DYN
done

cd utils
gcc av_time.c -o av
cd ..
./utils/av $REPORT_FILE STATIC DYNAMIC >> $REPORT_FILE

diff $OUTPUT_FILE_ST $OUTPUT_FILE_DYN >> $DIFFS_FILE

tail --lines=2 $REPORT_FILE

if [ -s $DIFFS_FILE ]
then
    echo "DIFFS WERE FOUND!!!"
    cat $DIFFS_FILE
    exit 1
else
    echo "CONGRATULATIONS - outputs are equal!"
    exit 0
fi
