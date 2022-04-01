#include <gtest/gtest.h>

extern "C" {
    #include "similarity.h"
}

#define FAIL_T 0
#define SUCCESS_T 1

TEST(print_vector, all) {
    double v[] = {1.0, 1.4, 1.2};
    EXPECT_EQ(print_vector(NULL, NULL, 0), FAIL_T);
    EXPECT_EQ(print_vector(stdout, NULL, 0), FAIL_T);
    EXPECT_EQ(print_vector(NULL, v, 3), FAIL_T);
    size_t STR = 100;
    char out_str[STR];
    FILE* out = fmemopen(out_str, STR, "w");
    EXPECT_EQ(print_vector(out, v, 3), SUCCESS_T);
    fclose(out);
    char *out_ex = (char *)"(1.000, 1.400, 1.200)\n";
    //printf("\n%s\n%s\n", out_ex, out_str);
    EXPECT_EQ(strcmp(out_ex, out_str), 0);
}

TEST(Result_print, all) {
    const size_t N = 3;
    double **v = (double **)calloc(N, sizeof(double *));
    for (size_t i = 0; i < N; ++i) {
        v[i] = (double *)malloc(N * sizeof(double));
        v[i][0] = 1.0;
        v[i][1] = 1.4;
        v[i][2] = 1.2;
    }
    Result res = {.count_t = 3, .dist_t = 2, .res_t = v};
    EXPECT_EQ(Result_print(NULL, NULL, N), FAIL_T);
    EXPECT_EQ(Result_print(NULL, &res, N), FAIL_T);
    EXPECT_EQ(Result_print(stdout, NULL, N), FAIL_T);
    size_t STR = 200;
    char out_str[STR];
    FILE* out = fmemopen(out_str, STR, "w");
    EXPECT_EQ(Result_print(out, &res, N), SUCCESS_T);
    fclose(out);
    char *out_ex = (char *)"(1.000, 1.400, 1.200)\n"
                           "(1.000, 1.400, 1.200)\n"
                           "(1.000, 1.400, 1.200)\n"
                           "with distance = 2.000\n";
    //printf("\n%s\n%s\n", out_ex, out_str);
    EXPECT_EQ(strcmp(out_ex, out_str), 0);
    for (size_t i = 0; i < N; ++i) {
        free(v[i]);
    }
    free(v);
}

TEST(cos_dist, all) {
    size_t N = 3;
    double a[] = {1.0, 1.4, 1.2};
    double b[] = {0, 0, 0};
    double c[] = {-1.0, -1.4, -1.2};
    double d[] = {0, 0, 1};
    double e[] = {0, 1, 0};
    EXPECT_EQ(cos_dist(NULL, a, N), -3);
    EXPECT_EQ(cos_dist(b, a, 0), -3);
    EXPECT_EQ(cos_dist(a, a, N), 1.0);
    EXPECT_EQ(cos_dist(a, b, N), -2.0);
    EXPECT_EQ(cos_dist(a, c, N), -1.0);
    EXPECT_EQ(cos_dist(d, e, N), 0);
}

TEST(find_nearest, all) {
    const size_t N = 3;
    const size_t M = 3;
    double **v = (double **)calloc(M, sizeof(double *));
    for (size_t i = 1; i <= N; ++i) {
        v[i - 1] = (double *)malloc(N * sizeof(double));
        v[i - 1][0] = i * 1.0;
        v[i - 1][1] = i * 1.4;
        v[i - 1][2] = i * 1.2;
    }
    double e[] = {0, 1, 0};
    Result res = {.count_t = 0, .dist_t = DBL_MAX, .res_t = NULL};
    EXPECT_EQ(find_nearest(NULL, v, M, N, &res), FAIL_T);
    EXPECT_EQ(find_nearest(e, NULL, M, N, &res), FAIL_T);
    EXPECT_EQ(find_nearest(e, v, 0, N, &res), FAIL_T);
    EXPECT_EQ(find_nearest(e, v, M, 0, &res), FAIL_T);
    EXPECT_EQ(find_nearest(e, v, M, N, NULL), FAIL_T);

    size_t STR = 100;
    char str[STR];
    FILE* out;
    int res_count = 0;
    out = fmemopen(str, STR, "w");
    res_count = find_nearest(e, v, M, N, &res);
    EXPECT_EQ(res_count, res.count_t);

    //fprintf(out, "Null vector found on line 0\n");
    Result_print(out, &res, N);
    fclose(out);

    char *str_ex = (char *)"(1.000, 1.400, 1.200)\n"
                           "(2.000, 2.800, 2.400)\n"
                           "(3.000, 4.200, 3.600)\n"
                           "with distance = 0.667\n";
    printf("\n%s\n\n%s\n", str_ex, str);
    EXPECT_EQ(strcmp(str, str_ex), 0);
//    free(res.res_t);

//    for (size_t i = 0; i < N - 1; ++i) {
//        v[i][0] = (i + 1) * 10.0;
//        v[i][1] = (i + 1) * 1.4;
//        v[i][2] = (i + 1) * 1.2;
//    }
//    res_count = find_nearest(e, v, M, N, &res);
//    char str2[STR];
//    out = fmemopen(str2, STR, "w");
//    Result_print(out, &res, N);
//    fclose(out);
//
//    char *str_ex2 = (char *)"(10.000, 1.400, 1.200)\n"
//                            "(20.000, 2.800, 2.400)\n"
//                            "with distance = 0.138\n";
//
//    //printf("\n%s\n\n%s\n", str_ex2, str2);
//    EXPECT_EQ(strcmp(str2, str_ex2), 0);

    for (size_t i = 0; i < N; ++i) {
        free(v[i]);
    }
    free(v);
    free(res.res_t);
}

TEST(get_vector, all) {
    size_t N = 3;
    char * in_str = (char *)"10.000 1.400 1.200\n";
    double v_ex[] = {10, 1.4, 1.2};
    FILE* in = fmemopen(in_str, strlen(in_str), "r");
    double *v = (double *)malloc(sizeof(double) * N);

    EXPECT_EQ(get_vector(NULL, v, N), FAIL_T);
    EXPECT_EQ(get_vector(in, v, N), N);

    for (size_t i = 0; i < N; ++i) {
        EXPECT_EQ(v[i], v_ex[i]);
    }

    fclose(in);
    free(v);
}

//TEST(get_array, all){
//    size_t N = 3;
//    size_t M = 3;
//    char * in_str = (char *)"1.0 1.0 1.0\n"
//                            "1.0 1.5 1.0\n"
//                            "-1.0 1.5 1.0\n";
//    FILE* in = fmemopen(in_str, strlen(in_str), "r");
//    double **v = (double **)calloc(M, sizeof(double *));
//    double **v_ex = (double **)calloc(M, sizeof(double *));
//    for (size_t i = 0; i < N; ++i) {
//        v[i] = (double *)malloc(N * sizeof(double));
//        v_ex[i] = (double *)malloc(N * sizeof(double));
//    }
//    v_ex[0][0] = v_ex[0][1] = v_ex[0][2] = v_ex[1][0] = v_ex[1][2] = v_ex[2][2] = 1;
//    v_ex[1][1] = v_ex[2][1] = 1.5;
//    v_ex[2][0] = -1;
//
//    EXPECT_EQ(get_array(NULL, v, M, N), FAIL_T);
//    EXPECT_EQ(get_array(in, v, M, N), M);
//
//    for (size_t i = 0; i < M; ++i) {
//        for (size_t j = 0; j < N; ++j) {
//            EXPECT_EQ(v[i][j], v_ex[i][j]);
//        }
//    }
//
//    fclose(in);
//    for (size_t i = 0; i < N; ++i) {
//        free(v[i]);
//        free(v_ex[i]);
//    }
//    free(v);
//    free(v_ex);
//}

//TEST(get_run, all) {
//    size_t N = 3;
//    size_t M = 3;
//    char * in_str = (char *)"1.0 1.0 1.0\n"
//                            "1.0 1.5 1.0\n"
//                            "-1.0 1.5 1.0\n";
//    FILE* in = fmemopen(in_str, strlen(in_str), "r");
//    double **v = (double **)calloc(M, sizeof(double *));
//    double **v_ex = (double **)calloc(M, sizeof(double *));
//    for (size_t i = 0; i < N; ++i) {
//    v[i] = (double *)malloc(N * sizeof(double));
//    v_ex[i] = (double *)malloc(N * sizeof(double));
//    }
//    v_ex[0][0] = v_ex[0][1] = v_ex[0][2] = v_ex[1][0] = v_ex[1][2] = v_ex[2][2] = 1;
//    v_ex[1][1] = v_ex[2][1] = 1.5;
//    v_ex[2][0] = -1;
//
//    char * in_str_vect = (char *)"10.000 1.400 1.200\n";
//    FILE* in_vect = fmemopen(in_str, strlen(in_str_vect), "r");
//    double *vect = (double *)malloc(sizeof(double) * N);
//
//    size_t STR = 100;
//    char str[STR];
//    FILE* out = fmemopen(str, STR, "w");
//
//    EXPECT_EQ(get_run(in_vect, NULL, in, vect, v, M, N), FAIL_T);
//    EXPECT_EQ(get_run(in_vect, out, NULL, vect, v, M, N), FAIL_T);
//    EXPECT_EQ(get_run(NULL, out, in, vect, v, M, N), FAIL_T);
//    EXPECT_EQ(get_run(in_vect, out, in, vect, v, M, N), SUCCESS_T);
//
//    fclose(in);
//    fclose(in_vect);
//    fclose(out);
//    for (size_t i = 0; i < N; ++i) {
//        free(v[i]);
//        free(v_ex[i]);
//    }
//    free(v);
//    free(v_ex);
//    free(vect);
//}

TEST(run_cos, all) {
    char * in_vect = (char *)"10.000 1.400 1.200\n";
    FILE* in = fmemopen(in_vect, strlen(in_vect), "r");
    size_t STR = 100;
    char out_str[STR];
    FILE* out = fmemopen(out_str, STR, "w");

    char *arr_input = (char *)"array_input.txt";
    char *in_str_arr = (char *)"3 3\n"
                                "1.0 1.0 1.0\n"
                                "1.0 1.5 1.0\n"
                                "-1.0 1.5 1.0\n";
    FILE* tmp = fopen(arr_input, "w");

    fprintf(tmp, "%s", in_str_arr);

    fclose(tmp);

    EXPECT_EQ(run_cos(NULL, out, arr_input), FAIL_T);
    EXPECT_EQ(run_cos(in, NULL, arr_input), FAIL_T);
    EXPECT_EQ(run_cos(in, out, NULL), FAIL_T);
    EXPECT_EQ(run_cos(in, out, arr_input), SUCCESS_T);
    fclose(in);
    fclose(out);
    remove(arr_input);
}
