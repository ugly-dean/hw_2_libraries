#include "similarity.h"

#include "float.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"

#define FILE_ERROR "IN, OUT or ARRAY_INPUT is NULL\n"
#define OUT_ERROR "FILE* out is NULL\n"
#define VECTOR_ERROR "double *vector is NULL\n"
#define RES_ERROR "double *res is NULL\n"
#define NULL_VECTOR_MSG "Null vector found on line %zu\n"
#define COS_DIST_VECTOR_ERROR -3
#define COS_DIST_VECTOR_NULL -2
#define RESULT_DIST_FORMAT "with distance = %.3lf\n"
#define DBL_FORMAT "%.3lf, "
#define DBL_FORMAT_LAST "%.3lf)\n"
#define VECTOR_SIZE_ERROR "Size of vector N = 0!\n"
#define ERRFILE stderr

#define FAIL 0
#define SUCCESS 1

char *__FILE_NAME_SIMILARITY_THREADS;

int get_run(FILE *in, FILE *out, FILE *array_input, double *vector,
            double **array, size_t M, size_t N);

int found_less_dist(result_t *res, long double dist, double *array);

int found_same_dist(result_t *res, double *array);

int compute_cos(double *vector, double *array, size_t N, result_t *res);

int get_array(FILE *array_input, double **array, size_t M, size_t N);

int malloc_run_free(FILE *in, FILE *out, FILE *array_input) {
  size_t N = 0;  // size of vector (cols)
  size_t M = 0;  // size of array (rows)
  fscanf(array_input, "%zu %zu\n", &M, &N);
  int status = SUCCESS;
  double *vector = (double *)calloc(N, sizeof(double));
  double **array = (double **)calloc(M, sizeof(double *));
  if (!vector || !array) {
    status = FAIL;
  }
  if (status) {
    for (size_t i = 0; i < M; ++i) {
      array[i] = (double *)calloc(N, sizeof(double));
    }
    status = get_run(in, out, array_input, vector, array, M, N);
    for (size_t i = 0; i < M; ++i) {
      free(array[i]);
    }
  }
  free(array);
  free(vector);
  return status;
}

int copy_file_name(char *array_file) {
  __FILE_NAME_SIMILARITY_THREADS = (char *)malloc(strlen(array_file) + 1);
  if (!__FILE_NAME_SIMILARITY_THREADS) {
    return FAIL;
  }
  strncpy(__FILE_NAME_SIMILARITY_THREADS, array_file, strlen(array_file) + 1);
  return SUCCESS;
}

int run_cos(FILE *in, FILE *out, char *array_file) {
  if (!array_file) {
    return FAIL;
  }
  FILE *array_input = fopen(array_file, "r");
  if (!in || !out || !array_input) {
    fprintf(ERRFILE, "%s", FILE_ERROR);
    if (array_input) {
      fclose(array_input);
    }
    return FAIL;
  }
  int status = copy_file_name(array_file);
  if (status) {
    status = malloc_run_free(in, out, array_input);
  }
  free(__FILE_NAME_SIMILARITY_THREADS);
  fclose(array_input);
  return status;
}

int get_array_and_vector(FILE *in, FILE *array_input, double *vector,
                         double **array, size_t M, size_t N) {
  if (!get_vector(in, vector, N) || vector_is_null(vector, N) == 1) {
    return FAIL;
  }
  if (!get_array(array_input, array, M, N)) {
    return FAIL;
  }
  return SUCCESS;
}

int get_result(FILE *out, double *vector, double **array, size_t M, size_t N) {
  int status = SUCCESS;
  result_t res = {.count_t = 0, .dist_t = DBL_MAX, .res_t = NULL};
  res.res_t = (double **)malloc(sizeof(double *));
  if (!res.res_t) {
    status = FAIL;
  }
  if (!status || !find_nearest(vector, array, M, N, &res) ||
      !result_print(out, &res, N)) {
    status = FAIL;
  }
  free(res.res_t);
  return status;
}

int get_run(FILE *in, FILE *out, FILE *array_input, double *vector,
            double **array, size_t M, size_t N) {
  int status = get_array_and_vector(in, array_input, vector, array, M, N);
  if (status) {
    status = get_result(out, vector, array, M, N);
  }
  return status;
}

int get_vector(FILE *in, double *vector, size_t N) {
  if (!in || !vector || !N) {
    return FAIL;
  }
  for (size_t i = 0; i < N; ++i) {
    fscanf(in, "%lf", vector + i);
  }
  return N;
}

int get_array(FILE *array_input, double **array, size_t M, size_t N) {
  if (!array_input || !array || !M || !N) {
    return FAIL;
  }
  for (size_t i = 0; i < M; ++i) {
    for (size_t j = 0; j < N; ++j) {
      fscanf(array_input, "%lf", array[i] + j);
    }
  }
  return M;
}

int find_nearest(double *vector, double **array, size_t M, size_t N,
                 result_t *res) {
  if (!vector || !array || !M || !N || !res) {
    return FAIL;
  }
  for (size_t i = 0; i < M; ++i) {
    if (!compute_cos(vector, array[i], N, res)) {
      printf(NULL_VECTOR_MSG, i);
    }
  }
  return res->count_t;
}

int compute_cos(double *vector, double *array, size_t N, result_t *res) {
  double dist = cos_dist(vector, array, N);
  int st = SUCCESS;
  if (dist < -1) {
    return FAIL;
  }
  if (fabs(dist) < fabs(res->dist_t)) {
    st = found_less_dist(res, dist, array);
  } else if (dist == res->dist_t) {
    st = found_same_dist(res, array);
  }
  return st;
}

double cos_dist(double *left, double *right, size_t N) {
  if (!left || !right || !N) {
    return COS_DIST_VECTOR_ERROR;
  }
  double mult = 0;
  double metric_left = 0;
  double metric_right = 0;
  for (size_t i = 0; i < N; ++i) {
    mult += (left[i] * right[i]);
    metric_left += (left[i] * left[i]);
    metric_right += (right[i] * right[i]);
  }
  metric_left = sqrt(metric_left);
  metric_right = sqrt(metric_right);
  if (!metric_left || !metric_right) {
    return COS_DIST_VECTOR_NULL;
  }
  return mult / (metric_left * metric_right);
}

int found_less_dist(result_t *res, long double dist, double *array) {
  res->dist_t = dist;
  res->count_t = 1;
  res->res_t = (double **)realloc(res->res_t, sizeof(double *));
  if (!res->res_t) {
    return FAIL;
  }
  *(res->res_t) = array;
  return SUCCESS;
}

int found_same_dist(result_t *res, double *array) {
  res->count_t++;
  res->res_t = (double **)realloc(res->res_t, sizeof(double *) * res->count_t);
  if (!res->res_t) {
    return FAIL;
  }
  res->res_t[res->count_t - 1] = array;
  return SUCCESS;
}

int result_print(FILE *out, result_t *res, size_t N) {
  if (!out) {
    fprintf(ERRFILE, "%s", OUT_ERROR);
    return FAIL;
  }
  if (!res) {
    fprintf(ERRFILE, "%s", RES_ERROR);
    return FAIL;
  }
  if (!N) {
    fprintf(ERRFILE, "%s", VECTOR_SIZE_ERROR);
    return FAIL;
  }
  for (size_t i = 0; i < res->count_t; ++i) {
    print_vector(out, res->res_t[i], N);
  }
  fprintf(out, RESULT_DIST_FORMAT, res->dist_t);
  return SUCCESS;
}

int print_vector(FILE *out, double *vector, size_t N) {
  if (!out) {
    fprintf(ERRFILE, "%s", OUT_ERROR);
    return FAIL;
  }
  if (!vector) {
    fprintf(ERRFILE, "%s", VECTOR_ERROR);
    return FAIL;
  }
  if (!N) {
    fprintf(ERRFILE, "%s", VECTOR_SIZE_ERROR);
    return FAIL;
  }
  fprintf(out, "(");
  for (size_t i = 0; i < N - 1; ++i) {
    fprintf(out, DBL_FORMAT, vector[i]);
  }
  fprintf(out, DBL_FORMAT_LAST, vector[N - 1]);
  return SUCCESS;
}

int vector_is_null(double *vector, size_t N) {
  if (!vector || !N) {
    return -1;
  }
  double res = 0.0;
  for (size_t i = 0; i < N; ++i) {
    res += fabs(vector[i]);
  }
  return res == 0.0 ? 1 : 0;
}
