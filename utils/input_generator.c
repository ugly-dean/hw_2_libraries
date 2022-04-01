#include "stdio.h"
#include "stdlib.h"

#define FILE_NAME "../array_input.txt"

const size_t ROWS = 10000000;
const size_t COLS = 5;

int main() {
    FILE* out = fopen(FILE_NAME, "w");
    fprintf(out, "%zu %zu\n", ROWS, COLS);
    for (size_t i = 0; i < ROWS; ++i) {
        for (size_t j = 0; j < COLS; ++j) {
            fprintf(out, "%7.3f ", (float) rand() / (float)RAND_MAX * 1000.0);
        }
        fprintf(out, "\n");
    }
    fclose(out);
    return 0;
}