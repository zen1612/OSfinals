#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void show_memory(const char *who) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "echo '%s (PID %d)' && grep Vm /proc/%d/status", who, getpid(), getpid());
    system(cmd);
}

int main() {
    char *mem = malloc(4096);
    strcpy(mem, "test string");

    show_memory("Parent before fork");

    pid_t pid = fork();
    if (pid == 0) {
        show_memory("Child before write");

        mem[0] = 'X';  

        show_memory("Child after write");
        printf("Child modified: %s\n", mem);
        _exit(0);
    } else {
        wait(NULL);
        show_memory("Parent after child");
        printf("Parent sees: %s\n", mem);
    }

    free(mem);
    return 0;
}
