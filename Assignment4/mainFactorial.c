//Seung Hoon Lee Kim
//A01021720
//Assignment 4 - Factorial Forks


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 

void createProcess(int num);

int main() {
    int temp;
    printf("Enter the number you wish to get the factorial of\n"); //We do a scanf for our 
    scanf("%d", &temp);

    printf ("Main function of factorial:\n");
    createProcess(temp);

    return 0;
}

void createProcess(int num) {
    pid_t my_pid; //Define the pids for all of our functions
    pid_t my_father;
    pid_t new_pid;
    char str[10];

    sprintf(str, "%d", num);
    char * temp = str;
    char * runProgram = "./factorial"; //Line to change for the executable
    char * parameters[] = {"factorial", str, (char *) NULL };

    new_pid = fork();       // Another process is created here
    my_pid = getpid(); //Set our pid
    my_father = getppid(); //Set the father 

    if (new_pid > 0)    // The parent process
    {
        printf("PARENT PROCESS: pid %d, ppid %d\n\n", my_pid, my_father); 
        int status;
        
        pid_t child_pid = wait(&status);
        
        if (WIFEXITED(status))
        {
            printf("My child %d has finished with status %d\n\n",
                    child_pid, WEXITSTATUS(status));
        }
    }
    else if (new_pid == 0)      // A child process
    {
        printf("CHILD PROCESS: pid %d, ppid %d\n\n", my_pid, my_father);
        execvp(runProgram, parameters);
    }
    else
    {
        printf("ERROR: Could not complete the fork");
    }
}