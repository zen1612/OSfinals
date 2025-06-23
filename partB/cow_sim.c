#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main() {
    char *mem = malloc(4096);
    strcpy(mem, "test string");

    pid_t pid = fork();
    if (pid == 0) {
        printf("Child: %s\n", mem);
        mem[0] = 'X';
        printf("Child modifies to: %s\n", mem);
    } else {
        wait(NULL);
        printf("Parent: %s\n", mem);
    }

    free(mem);
    return 0;
}
