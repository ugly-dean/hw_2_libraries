#include "similarity.h"

#include "float.h"
#include "math.h"
#include "pthread.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include "sys/sysinfo.h"

#define FILE_ERROR "IN, OUT or ARRAY_INPUT is NULL\n"
#define OUT_ERROR "FILE* out is NULL\n"
#define VECTOR_ERROR "double *vector is NULL\n"
#define RES_ERROR "double *res is NULL\n"
#define DBL_FORMAT "%.3lf, "
#define DBL_FORMAT_LAST "%.3lf)\n"
#define ERRFLAG "Something wrong with treads!\n"
#define COS_DIST_VECTOR_ERROR -3
#define COS_DIST_VECTOR_NULL -2
#define RESULT_DIST_FORMAT "with distance = %.3lf\n"
#define VECTOR_SIZE_ERROR "Size of vector N = 0!\n"
#define ERRFILE stderr

#define FAIL 0
#define SUCCESS 1

char *__FILE_NAME_SIMILARITY_THREADS;

typedef struct Thread_read {
  FILE *in_t;
  int64_t pos_t;
  size_t from_t;
  size_t to_t;
  size_t N_t;
  double **arr_t;
} Thread_read;

typedef struct Thread_data {
  size_t from_t;
  size_t to_t;
  double *v_t;
  double **arr_t;
  size_t N_t;
  result_t *res_t;
} Thread_data;

int get_run(FILE *in, FILE *out, FILE *array_input, double *vector,
            double **array, size_t M, size_t N);

int found_less_dist(result_t *res, long double dist, double *array);

int found_same_dist(result_t *res, double *array);

int compute_cos(double *array, result_t *res, double dist);

int update_res(result_t *res, result_t *data);

int get_array(FILE *array_input, double **array, size_t M, size_t N);

int get_array_via_threads(FILE *array_input, double **array, size_t M,
                          size_t N);

int64_t file_size(FILE *file);

int final_dist_less(result_t *res, result_t *data);

int final_dist_same(result_t *res, result_t *data);

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
  int code = get_array_via_threads(array_input, array, M, N);
  // printf("Got array with exit code = %d\n", code);
  return code;
}

void *thread_read(void *thread_data) {
  Thread_read *data = (Thread_read *)thread_data;
  // printf(THREAD_START, data->from_t);
  fseek(data->in_t, data->pos_t, 0);
  for (size_t i = data->from_t; i < data->to_t; ++i) {
    for (size_t j = 0; j < data->N_t; ++j) {
      fscanf(data->in_t, "%lf", data->arr_t[i] + j);
    }
  }
  // printf(THREAD_FINISH, data->to_t);
  return NULL;
}

int get_array_via_threads(FILE *array_input, double **array, size_t M,
                          size_t N) {
  int64_t start_pos = ftell(array_input);
  // printf("start_pos = %ld\n", start_pos);
  int64_t fs = file_size(array_input);
  const size_t THREADS = get_nprocs();
  pthread_t threads[THREADS];
  Thread_read data[THREADS];
  const size_t bytes_per_thread = (fs - start_pos) / THREADS;
  const size_t lines_per_thread = M / THREADS;
  int error;
  int status = SUCCESS;
  for (size_t i = 0; i < THREADS && status == SUCCESS; ++i) {
    data[i].in_t = fopen(__FILE_NAME_SIMILARITY_THREADS, "r");
    data[i].pos_t = i * bytes_per_thread + start_pos;
    data[i].from_t = i * lines_per_thread;
    if (i < THREADS - 1) {
      data[i].to_t = (i + 1) * lines_per_thread;
    } else {
      data[i].to_t = M;
    }
    data[i].N_t = N;
    data[i].arr_t = array;
    error = pthread_create(threads + i, NULL, thread_read, &data[i]);
    if (error != 0) {
      status = FAIL;
    }
  }
  for (size_t i = 0; i < THREADS; ++i) {
    error = pthread_join(threads[i], 0);
    if (error != 0) {
      status = FAIL;
    }
    fclose(data[i].in_t);
  }
  return status == FAIL ? FAIL : (int)M;
}

void *thread_compute_cos(void *thread_data) {
  Thread_data *data = (Thread_data *)thread_data;
  // printf("From %zu to %zu\n", data->from_t, data->to_t);
  for (size_t i = data->from_t; i < data->to_t; ++i) {
    double dist = cos_dist(data->v_t, data->arr_t[i], data->N_t);
    //        if (dist < 0.464) {
    //            printf("Array: i = %zu, dist = %.4lf\n", i, dist);
    //        }
    compute_cos(data->arr_t[i], data->res_t, dist);
  }
  // printf("Found min: dist = %lf, address = %ld\n", data->res_t->dist_t,
  // (long)data->res_t);
  return NULL;
}

int find_nearest(double *vector, double **array, size_t M, size_t N,
                 result_t *res) {
  if (!vector || !array || !M || !N || !res) {
    return FAIL;
  }
  const size_t THREADS = get_nprocs();
  pthread_t threads[THREADS];
  Thread_data data[THREADS];
  const size_t count_per_thread = M / THREADS;
  for (size_t i = 0; i < THREADS; ++i) {
    data[i].from_t = i * count_per_thread;
    if (i < THREADS - 1) {
      data[i].to_t = (i + 1) * count_per_thread;
    } else {
      data[i].to_t = M;
    }
    data[i].v_t = vector;
    data[i].arr_t = array;
    data[i].N_t = N;
    data[i].res_t = (result_t *)malloc(sizeof(result_t));
    data[i].res_t->dist_t = DBL_MAX;
    data[i].res_t->count_t = 0;
    data[i].res_t->res_t = (double **)malloc(sizeof(double *));
    pthread_create(&threads[i], NULL, thread_compute_cos, &data[i]);
  }
  for (size_t i = 0; i < THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  for (size_t i = 0; i < THREADS; i++) {
    //    printf("%zu : dist = %lf, count = %zu\n", i, data[i].res_t->dist_t,
    //           data[i].res_t->count_t);
    update_res(res, data[i].res_t);
    free(data[i].res_t->res_t);
    free(data[i].res_t);
  }
  //    printf("Found nearest with count = %zu\n", res->count_t);
  return res->count_t;
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

int64_t file_size(FILE *file) {
  struct stat stats;
  int fd = fileno(file);
  if (fstat(fd, &stats) != -1) {
    return stats.st_size;
  }
  return -1;
}

int update_res(result_t *res, result_t *data) {
  if (!res || !data) {
    return FAIL;
  }
  if (data->dist_t < res->dist_t) {
    return final_dist_less(res, data);
  } else if (data->dist_t == res->dist_t) {
    return final_dist_same(res, data);
  }
  return SUCCESS;
}

int final_dist_less(result_t *res, result_t *data) {
  res->dist_t = data->dist_t;
  res->count_t = data->count_t;
  res->res_t = (double **)realloc(res->res_t, sizeof(double *) * data->count_t);
  if (!res->res_t) {
    return FAIL;
  }
  for (size_t i = 0; i < data->count_t; ++i) {
    res->res_t[i] = data->res_t[i];
  }
  return SUCCESS;
}

int final_dist_same(result_t *res, result_t *data) {
  if (!res || !res->res_t || !data || !data->res_t) {
    return FAIL;
  }
  int status = SUCCESS;
  for (size_t i = 0; i < data->count_t && status == SUCCESS; ++i) {
    size_t old_size = res->count_t;
    res->count_t += data->count_t;
    res->res_t =
        (double **)realloc(res->res_t, sizeof(double *) * res->count_t);
    if (!res->res_t) {
      status = FAIL;
    }
    for (size_t j = 0; j < data->count_t; ++j) {
      res->res_t[j + old_size] = data->res_t[j];
    }
  }
  return status;
}

int compute_cos(double *array, result_t *res, double dist) {
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

int found_less_dist(result_t *res, long double dist, double *array) {
  res->dist_t = dist;
  res->count_t = 1;
  free(res->res_t);
  res->res_t = (double **)malloc(sizeof(double *));
  if (!res->res_t) {
    return FAIL;
  }
  *(res->res_t) = array;
  return SUCCESS;
}

int found_same_dist(result_t *res, double *array) {
  res->count_t++;
  double **tmp =
      (double **)realloc(res->res_t, sizeof(double *) * res->count_t);
  if (!tmp) {
    return FAIL;
  }
  res->res_t = tmp;
  res->res_t[res->count_t - 1] = array;
  return SUCCESS;
}
