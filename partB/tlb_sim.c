#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define SIZE (512 * 1024 * 1024)
#define PAGE_SIZE 4096

double get_time_diff(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) + 
           (end->tv_nsec - start->tv_nsec) / 1e9;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || (argv[1][0] != 'r' && argv[1][0] != 's')) {
        fprintf(stderr, "Usage: %s [s|r]\n", argv[0]);
        return 1;
    }

    size_t steps = SIZE / sizeof(int);
    int *arr = malloc(steps * sizeof(int));
    if (!arr) {
        perror("malloc failed");
        return 1;
    }

    int stride = PAGE_SIZE / sizeof(int);
    int pages = steps / stride;
    srand(time(NULL));

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < steps; i += stride) {
        int index = (argv[1][0] == 'r') ? (rand() % pages) * stride : i;
        arr[index]++;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("%s access time: %.6f seconds\n",
           (argv[1][0] == 'r') ? "Random" : "Sequential",
           get_time_diff(&start, &end));

    free(arr);
    return 0;
}


//gcc -O2 -o tlb_test tlb_test.c
//perf stat -e dTLB-loads,dTLB-load-misses ./tlb_test s  # sequential
//perf stat -e dTLB-loads,dTLB-load-misses ./tlb_test r  # random
