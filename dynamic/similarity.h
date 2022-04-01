#ifndef HW_2_LIBRARIES_DYNAMIC_SIMILARITY_H_
#define HW_2_LIBRARIES_DYNAMIC_SIMILARITY_H_

#include "stdio.h"

typedef struct result_t {
  size_t count_t;
  double dist_t;
  double **res_t;
} result_t;

int result_print(FILE *out, result_t *res, size_t N);

int run_cos(FILE *in, FILE *out, char *array_file);

int get_vector(FILE *in, double *vector, size_t N);

double cos_dist(double *left, double *right, size_t N);

int find_nearest(double *vector, double **array, size_t M, size_t N,
                 result_t *res);

int print_vector(FILE *out, double *vector, size_t N);

int vector_is_null(double *vector, size_t N);

#endif  // HW_2_LIBRARIES_DYNAMIC_SIMILARITY_H_
