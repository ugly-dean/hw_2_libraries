#include "stdio.h"
#include "stdlib.h"

#define NO_INPUT    "Invalid args for av_time\n"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stdout, "%s", NO_INPUT);
    }
    FILE* in = fopen(argv[1], "r");
    size_t count = 0;
    fscanf(in, "%zu", &count);
    float av_st = 0;
    for (size_t i = 0; i < count; ++i) {
        float tmp;
        fscanf(in, "%f", &tmp);
        av_st += tmp;
    }
    av_st = av_st / (float)count;

    fscanf(in, "%zu", &count);
    float av_dyn = 0;
    for (size_t i = 0; i < count; ++i) {
        float tmp;
        fscanf(in, "%f", &tmp);
        av_dyn += tmp;
    }
    av_dyn = av_dyn / (float)count;

    fclose(in);
    in = fopen(argv[1], "a");
    fprintf(in, "%10s: %f\n", argv[2], av_st);
    fprintf(in, "%10s: %f\n", argv[3], av_dyn);
    fclose(in);

    return 0;
}