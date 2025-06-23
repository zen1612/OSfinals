#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <time.h>

#define MB (1024 * 1024)
#define PAGE_SIZE 4096

volatile int running = 1;

void signal_handler(int sig) {
    running = 0;
    printf("\nTerminating\n");
}

void get_memory_info() {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        printf("Total RAM: %lu MB\n", info.totalram / MB);
        printf("Free RAM: %lu MB\n", info.freeram / MB);
        printf("Will allocate: %lu MB (150%% of RAM)\n", (info.totalram * 3 / 2) / MB);
    }
}

int main(int argc, char *argv[]) {

    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    get_memory_info();
    
    struct sysinfo info;
    sysinfo(&info);
    
    // allocates 150% of total ram to force swapping
    size_t target_size = info.totalram * 3 / 2;
    size_t chunk_size = 64 * MB;
    int num_chunks = target_size / chunk_size;
    
    printf("\nAllocating %d chunks of %zu MB each...\n", num_chunks, chunk_size / MB);
    
    char **chunks = malloc(num_chunks * sizeof(char*));
    if (!chunks) {
        perror("malloc failed");
        return 1;
    }
    
    // allocates all memory
    for (int i = 0; i < num_chunks && running; i++) {
        chunks[i] = malloc(chunk_size);
        if (!chunks[i]) {
            printf("Allocation failed at chunk %d\n", i);
            num_chunks = i;
            break;
        }
        
        // touch every page to force physical allocation
        for (size_t j = 0; j < chunk_size; j += PAGE_SIZE) {
            chunks[i][j] = (char)(i + j / PAGE_SIZE);
        }
        
        printf("Allocated chunk %d/%d \n", i + 1, num_chunks);
        usleep(50000); // 50ms delay
    }
    
    
    srand(time(NULL));
    unsigned long iterations = 0;
    
    // random access to cause thrashing
    while (running) {
        // accesses random chunks in random order
        for (int access = 0; access < 10 && running; access++) {
            int chunk_idx = rand() % num_chunks;
            if (!chunks[chunk_idx]) continue;
            
            // accesses random pages within the chunk
            for (int page = 0; page < 100; page++) {
                size_t offset = (rand() % (chunk_size / PAGE_SIZE)) * PAGE_SIZE;
                volatile char temp = chunks[chunk_idx][offset];
                chunks[chunk_idx][offset] = temp + 1;
            }
        }
        
        iterations++;
        if (iterations % 100 == 0) {
            printf("Ctrl+C to stop\n");
        }
        
        usleep(1000); 
    }
    
    for (int i = 0; i < num_chunks; i++) {
        if (chunks[i]) {
            free(chunks[i]);
        }
    }
    free(chunks);
    
    printf("Done.\n");
    return 0;
}
