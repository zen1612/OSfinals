#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define SIZE (512 * 1024 * 1024)  // 512MB
#define PAGE_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc < 2 || (argv[1][0] != 'r' && argv[1][0] != 's')) {
        fprintf(stderr, "Usage: %s [s|r] (s = sequential, r = random)\n", argv[0]);
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

    for (int i = 0; i < steps; i += stride) {
        int index = (argv[1][0] == 'r') ? (rand() % pages) * stride : i;
        arr[index]++;
    }

    free(arr);
    return 0;
}

//gcc -O2 -o tlb_test tlb_test.c
//perf stat -e dTLB-loads,dTLB-load-misses ./tlb_test s  # sequential
//perf stat -e dTLB-loads,dTLB-load-misses ./tlb_test r  # random
