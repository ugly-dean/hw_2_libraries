#!/bin/bash

TESTING_OPT=""
SANITIZE_OPT=""
COVERAGE_OPT=""
LINTER_OPT=""
PARALLEL_OPT=""

while getopts "ptscl" flag;
do
    case "${flag}" in
        p) PARALLEL_OPT="-DPARALLEL=ON";;
        t) TESTING_OPT="-DTEST=ON";;
        s) SANITIZE_OPT="-DSANITIZE=ON";;
        c) COVERAGE_OPT="-DCOVERAGE=ON";;
        l) LINTER_OPT="-DLINTERS-CHECK=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON";;
        *) echo "Usage options -s SANITIZE or -t TESTING" && exit 1
    esac
done

BUILD_DIR="./build"
LOG_FILE="./tmp/build.log"
TMP_DIR="./tmp"

rm -rf $BUILD_DIR
mkdir -p $TMP_DIR
touch $LOG_FILE
mkdir $BUILD_DIR && echo "Creating $BUILD_DIR directory"
cp ./array_input.txt $BUILD_DIR

CMAKE_KEYS="$TESTING_OPT $SANITIZE_OPT $COVERAGE_OPT $LINTER_OPT $PARALLEL_OPT"
echo "CMakeFile.txt will execute with $CMAKE_KEYS options"

echo "Executing CMakeFile.txt"
cmake $CMAKE_KEYS -B $BUILD_DIR -S . >> $LOG_FILE || (echo "Error, check $LOG_FILE")

echo "Build targets with make"
make --directory=$BUILD_DIR >> $LOG_FILE || (echo "Error, check $LOG_FILE")

echo "REBUILD FINISHED!"