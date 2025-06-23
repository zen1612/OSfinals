#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define SIZE (100 * 1024 * 1024) // 100MB
#define PAGE_SIZE 4096

int main() {
    //page fault simulator
    printf("\ncreating minor page faults using malloc()\n");
    char *malloc_mem = malloc(SIZE);
    if (!malloc_mem) {
        perror("malloc failed");
        return 1;
    }
    for (size_t i = 0; i < SIZE; i += PAGE_SIZE) {
        malloc_mem[i] = 1;
    }
    free(malloc_mem);

    char *anon_mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (anon_mem == MAP_FAILED) {
        perror("mmap anonymous failed");
        return 1;
    }
    for (size_t i = 0; i < SIZE; i += PAGE_SIZE) {
        anon_mem[i] = 1;
    }
    munmap(anon_mem, SIZE);

    printf("\ncreating major page faults using mmap()\n");

    // creates a test file and fill with data
    int fd = open("testfile", O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open failed");
        return 1;
    }

    char *buffer = malloc(PAGE_SIZE);
    memset(buffer, 0, PAGE_SIZE);
    for (size_t i = 0; i < SIZE / PAGE_SIZE; i++) {
        if (write(fd, buffer, PAGE_SIZE) != PAGE_SIZE) {
            perror("write failed");
            close(fd);
            free(buffer);
            return 1;
        }
    }
    free(buffer);
    fsync(fd);
    close(fd);  // flush and close

    // drops system cache with root access
    int ret = system("sudo sh -c 'sync; echo 3 > /proc/sys/vm/drop_caches'");
    if (ret != 0) {
        fprintf(stderr, "Failed to drop caches\n");
    }

    // reopens the file in read only
    fd = open("testfile", O_RDONLY);
    if (fd == -1) {
        perror("reopen failed");
        return 1;
    }

    char *file_mem = mmap(NULL, SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_mem == MAP_FAILED) {
        perror("mmap file failed");
        close(fd);
        return 1;
    }

    // accesses every page to trigger major page faults
    for (size_t i = 0; i < SIZE; i += PAGE_SIZE) {
        volatile char temp = file_mem[i]; 
    }

    munmap(file_mem, SIZE);
    close(fd);
    unlink("testfile");

    return 0;
}
