#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define SIZE 1024*1024*512 
#define PAGE_SIZE 4096 

int main(int argc, char *argv[]) {
    int *arr = malloc(SIZE);
    int steps = SIZE / sizeof(int);
    srand(time(NULL));

    for (int i = 0; i < steps; i += (PAGE_SIZE / sizeof(int))) {
        int index = (argv[1][0] == 'r') ? (rand() % (steps / (PAGE_SIZE / sizeof(int)))) * (PAGE_SIZE / sizeof(int)) : i;
        arr[index]++;
    }

    free(arr);
    return 0;
}

//gcc -O2 -o tlb_test tlb_test.c
//perf stat -e dTLB-loads,dTLB-load-misses ./tlb_test s  # sequential
//perf stat -e dTLB-loads,dTLB-load-misses ./tlb_test r  # random
