#include "stdio.h"
#include "stdlib.h"
#include "float.h"

const size_t ROWS = 1000000;
const size_t COLS = 5;

int main() {
    FILE* out = fopen("../array_input.txt", "w");
    fprintf(out, "%zu %zu\n", ROWS, COLS);
    for (size_t i = 0; i < ROWS; ++i) {
        for (size_t j = 0; j < COLS; ++j) {
            fprintf(out, "%7.3f ", (float) rand() / RAND_MAX * 1000.0);
        }
        fprintf(out, "\n");
    }
    fclose(out);
    return 0;
}