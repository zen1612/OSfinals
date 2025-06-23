#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define SIZE (100 * 1024 * 1024) // 100MB
#define PAGE_SIZE 4096

int main() {

    printf("\ncreating minor page faults with malloc.\n");
    char *malloc_mem = malloc(SIZE);
    if (!malloc_mem) {
        perror("malloc failed");
        return 1;
    }

    printf("touching malloc memory\n");
    for (size_t i = 0; i < SIZE; i += PAGE_SIZE) {
        malloc_mem[i] = 1;
    }
    printf("malloc() access done.\n");

    printf("\ncreating major page faults with mmap()\n");
    char *mmap_mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mmap_mem == MAP_FAILED) {
        perror("mmap failed");
        free(malloc_mem);
        return 1;
    }

    printf("Touching mmap memory...\n");
    for (size_t i = 0; i < SIZE; i += PAGE_SIZE) {
        mmap_mem[i] = 1;
    }
    printf("mmap() access done.\n");

    // Cleanup
    free(malloc_mem);
    munmap(mmap_mem, SIZE);

    printf("\nFinished.\n");
    return 0;
}
// use /usr/bin/time -v ./pagefault_sim to see page fault stats
