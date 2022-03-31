#include "similarity.h"
#include "stdio.h"

#define IN_ERROR "FILE* in cannot be opened!\n"
#define OUT_ERROR "FILE* out cannot be opened!\n"
#define ARRAY_INPUT_ERROR "FILE* input_array cannot be opened!\n"
#define ARRAY_INPUT "array_input.txt"
#define RUN_TIME_ERROR "Something went wrong!\n"

int main() {
  FILE* in = stdin;
  FILE* out = stdout;
  FILE* array_input = fopen(ARRAY_INPUT, "r");
  int status = 0;
  if (array_input && in && out) {
    status = !run_cos(in, out, ARRAY_INPUT);
    if (status != 0) {
      fprintf(out, RUN_TIME_ERROR);
    }
  } else {
    if (!in) {
      fprintf(stdout, "%s", IN_ERROR);
    }
    if (!out) {
      fprintf(stdout, "%s", OUT_ERROR);
    }
    if (!array_input) {
      fprintf(stdout, "%s", ARRAY_INPUT_ERROR);
    }
    status = 1;
  }
  if (in) fclose(in);
  if (out) fclose(out);
  if (array_input) fclose(array_input);
  return status;
}
