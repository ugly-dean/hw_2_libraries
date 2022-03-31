#include "similarity.h"

#include "float.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"

#define FILE_ERROR "IN, OUT or ARRAY_INPUT is NULL\n"
#define OUT_ERROR "FILE* out is NULL\n"
#define VECTOR_ERROR "double *vector is NULL\n"
#define RES_ERROR "double *res is NULL\n"

#define FAIL 0
#define SUCCESS 1

int get_run(FILE *in, FILE *out, FILE *array_input, double *vector,
            double **array, size_t M, size_t N);

int found_less_dist(Result *res, long double dist, double *array);

int found_same_dist(Result *res, double *array);

int compute_cos(double *vector, double *array, size_t N, Result *res);

int get_array(FILE *array_input, double **array, size_t M, size_t N);

int run_cos(FILE *in, FILE *out, char *array_file) {
  if (!array_file) {
    return FAIL;
  }
  FILE *array_input = fopen(array_file, "r");
  if (!in || !out || !array_input) {
    fprintf(stdin, "%s", FILE_ERROR);
    if (array_input) fclose(array_input);
    return FAIL;
  }
  size_t N = 0;  // size of vector (cols)
  size_t M = 0;  // size of array (rows)
  fscanf(array_input, "%zu %zu", &M, &N);
  double *vector = (double *)calloc(N, sizeof(double));
  double **array = (double **)calloc(M, sizeof(double *));
  if (array) {
    for (size_t i = 0; i < M; ++i) {
      array[i] = (double *)calloc(N, sizeof(double));
    }
  }
  int res = FAIL;
  if (vector && array) {
    res = get_run(in, out, array_input, vector, array, M, N);
  }
  if (array) {
    for (size_t i = 0; i < M; ++i) {
      free(array[i]);
    }
  }
  free(array);
  free(vector);
  fclose(array_input);
  return res;
}

int get_run(FILE *in, FILE *out, FILE *array_input, double *vector,
            double **array, size_t M, size_t N) {
  if (!get_vector(in, vector, N) || vector_is_null(vector, N)) {
    return FAIL;
  }
  if (!get_array(array_input, array, M, N)) {
    return FAIL;
  }
  Result res = {.count_t = 0, .dist_t = DBL_MAX, .res_t = NULL};
  if (!find_nearest(vector, array, M, N, &res) || !Result_print(out, &res, N)) {
    free(res.res_t);
    return FAIL;
  }
  free(res.res_t);
  return SUCCESS;
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

#define NULL_VECTOR_MSG "Null vector found on line %zu\n"

int find_nearest(double *vector, double **array, size_t M, size_t N,
                 Result *res) {
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

int compute_cos(double *vector, double *array, size_t N, Result *res) {
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

#define COS_DIST_VECTOR_ERROR -3
#define COS_DIST_VECTOR_NULL -2

double cos_dist(double *a, double *b, size_t N) {
  if (!a || !b || !N) {
    return COS_DIST_VECTOR_ERROR;  // -3 - код ошибки (пустой вектор)
  }
  double mult = 0;
  double metric_a = 0;
  double metric_b = 0;
  for (size_t i = 0; i < N; ++i) {
    mult += (a[i] * b[i]);
    metric_a += (a[i] * a[i]);
    metric_b += (b[i] * b[i]);
  }
  metric_a = sqrt(metric_a);
  metric_b = sqrt(metric_b);
  //  printf("\na = %lf, b = %lf, ab = %lf\n\n", metric_a, metric_b, mult);
  if (!metric_a || !metric_b) {  // один из векторов нулевой;
                                 //      print_vector(stdout, a, N);
                                 //      print_vector(stdout, b, N);
                                 //      scanf("%lf", &mult);
    return COS_DIST_VECTOR_NULL;  // косинусное расстояние по модулю меньше 1 -
  }  // принимаем -2 за ошибку
  return mult / (metric_a * metric_b);
}

int found_less_dist(Result *res, long double dist, double *array) {
  res->dist_t = dist;
  res->count_t = 1;
  res->res_t = (double **)realloc(res->res_t, sizeof(double *));
  if (!res->res_t) {
    return FAIL;
  }
  //    double **tmp = (double **)realloc(res->res_t, sizeof(double *));
  //    if (!tmp) {
  //        return FAIL;
  //    }
  //    free(res->res_t);
  //    res->res_t = tmp;
  *(res->res_t) = array;
  return SUCCESS;
}

int found_same_dist(Result *res, double *array) {
  res->count_t++;
  res->res_t = (double **)realloc(res->res_t, sizeof(double *) * res->count_t);
  if (!res->res_t) {
    return FAIL;
  }
  res->res_t[res->count_t - 1] = array;
  return SUCCESS;
}

#define RESULT_DIST_FORMAT "with distance = %.3lf\n"

int Result_print(FILE *out, Result *res, size_t N) {
  if (out && res) {
    for (size_t i = 0; i < res->count_t; ++i) {
      print_vector(out, res->res_t[i], N);
    }
    fprintf(out, RESULT_DIST_FORMAT, res->dist_t);
    return SUCCESS;
  }
  if (!out) {
    fprintf(stdout, "%s", OUT_ERROR);
  }
  if (!res) {
    fprintf(stdout, "%s", RES_ERROR);
  }
  return FAIL;
}

#define DBL_FORMAT "%.3lf, "
#define DBL_FORMAT_LAST "%.3lf)\n"

int print_vector(FILE *out, double vector[], size_t N) {
  if (out && vector) {
    fprintf(out, "(");
    for (size_t i = 0; i < N - 1; ++i) {
      fprintf(out, DBL_FORMAT, vector[i]);
    }
    fprintf(out, DBL_FORMAT_LAST, vector[N - 1]);
    return SUCCESS;
  }
  if (!out) {
    fprintf(stdout, "%s", OUT_ERROR);
  }
  if (!vector) {
    fprintf(stdout, "%s", VECTOR_ERROR);
  }
  return FAIL;
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
