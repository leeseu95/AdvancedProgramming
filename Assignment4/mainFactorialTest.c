#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     /* Unix system functions */

int main()
{
    pid_t my_pid;
    pid_t my_father;
    pid_t new_pid;
    
    printf ("TESTING FORK\n");
    
    new_pid = fork();       // Another process is created here
    
    if (new_pid > 0)    // The parent process
    {
        printf("A am now the father process!\n");
    }
    else if (new_pid == 0)      // A child process
    {
        printf("A am a child, going to run another program\n");
        char * program = "./factorial";
        char * parameters[] = {"factorial", "5", (char *) NULL };
        
        // Change the code of this process for something else
        execvp(program, parameters);
        //execlp(program, "python3", "python_test.py", (char *) NULL);
        
        printf("This will NOT be executed!!\n");
    }
    else
    {
        printf("ERROR: Could not complete the fork");
    }
    
    my_pid = getpid();
    my_father = getppid();
    
    printf("Hello, I am process %d\n", my_pid);
    printf("My father is: %d\n", my_father);
    
    return 0;
}