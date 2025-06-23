
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE (100 * 1024 * 1024)
#define PAGE_SIZE 4096

int main() {
    // malloc() - causes MINOR page faults (memory from RAM)
    printf("malloc() - creating minor page faults\n");
    char *malloc_mem = malloc(SIZE);
    if (!malloc_mem) {
        perror("malloc failed");
        return 1;
    }
    
    for (size_t i = 0; i < SIZE; i += PAGE_SIZE) {
        malloc_mem[i] = 1;  // MINOR page faults
    }
    free(malloc_mem);
    
    // mmap() anonymous - causes MINOR page faults (memory from RAM)
    printf("mmap() anonymous - creating minor page faults\n");
    char *anon_mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, 
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (anon_mem == MAP_FAILED) {
        perror("mmap anonymous failed");
        return 1;
    }
    
    for (size_t i = 0; i < SIZE; i += PAGE_SIZE) {
        anon_mem[i] = 1;  // MINOR page faults
    }
    munmap(anon_mem, SIZE);
    
    // Create file for mmap
    int fd = open("testfile", O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        perror("open failed");
        return 1;
    }
    
    char *buffer = malloc(PAGE_SIZE);
    for (int i = 0; i < SIZE / PAGE_SIZE; i++) {
        write(fd, buffer, PAGE_SIZE);
    }
    free(buffer);
    
    // mmap() file - causes MAJOR page faults (data from disk)
    printf("mmap() file - creating major page faults\n");
    char *file_mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file_mem == MAP_FAILED) {
        perror("mmap file failed");
        close(fd);
        return 1;
    }
    close(fd);
    
    system("sync");
    system("echo 3 > /proc/sys/vm/drop_caches 2>/dev/null");
    
    for (size_t i = 0; i < SIZE; i += PAGE_SIZE) {
        volatile char temp = file_mem[i];  // MAJOR page faults (reads from disk)
    }
    
    munmap(file_mem, SIZE);
    unlink("testfile");
    
    return 0;
}
