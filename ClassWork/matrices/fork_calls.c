#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t my_pid;
    pid_t my_father;
    pid_t new_pid;

    printf("Testing Fork:\n");

    new_pid = fork();

    if(new_pid > 0) {
        printf("I am now the father process!\n");
    } else if (new_pid == 0) {
        printf("I am a child\n");
    } else {
        printf("ERROR : Could not complete the fork");
    }
    my_pid = getpid();
    my_father = getppid();

    printf("Hello, I am process: %d\n", my_pid);
    printf("Hello i am the father: %d\n", my_father);
    return 0;
}