#!/bin/bash

TMP_DIR="./tmp"
LOG_FILE="./tmp/clang-formating.log"
TMP_FORM_FILE="./tmp/formated-file"
mkdir -p $TMP_DIR
touch $LOG_FILE
touch $TMP_FORM_FILE

STYLE="Google"
FORMATING_FILES=("main.c" \
                 "static/*" \
                 "dynamic/*")
for file in ${FORMATING_FILES[*]}
do
    echo $file
    clang-format -style=$STYLE $file > $TMP_FORM_FILE || (echo "Error, check $LOG_FILE")
    diff $file $TMP_FORM_FILE
done