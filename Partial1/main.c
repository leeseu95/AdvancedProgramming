//Seung Hoon Lee Kim
//A01021720
//Partial 1
//Main Functions to call the ciphering/deciphering

//Libraries to include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

//Buffer Size
#define BUFFER_SIZE 20

///// FUNCTION DECLARATIONS /////
void createProcess(int num, char* textFile, char* key);
void userMenu();

//Main Function
int main()
{
    printf("\n\n\nProgram to Cipher/Decipher by Seung Lee\n\n");
    
    userMenu();

    return 0;
}

//Create Process Function
void createProcess(int num, char* textFile, char* key) {
    pid_t my_pid; //Define the pids for all of our functions
    pid_t my_father;
    pid_t new_pid;
    char str[10];
    char str2[10];
    char str3[10];

    sprintf(str, "%d", num);
    sprintf(str2, "%s", textFile);
    sprintf(str3, "%s", key);

    char * runProgram = "./vigenere"; //Line to change for the executable
    char * parameters[] = {"vigenere", str, str2, str3, (char *) NULL };

    new_pid = fork();       // Another process is created here
    my_pid = getpid(); //Set our pid
    my_father = getppid(); //Set the father 

    if (new_pid > 0)    // The parent process
    {
        printf("\n\nPARENT PROCESS: pid %d, ppid %d\n\n", my_pid, my_father); 
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

// Main loop for the parent process, listens for the user requests
// Receive: the arrays with the file descriptors for input and output
void userMenu()
{
    int number = 0;
    char key[BUFFER_SIZE];
    char text[BUFFER_SIZE];

    while (number != 3)
    {
        printf("\n\n\nPlease choose the option that you'd like to do\n1.Cipher\n2.Decipher\n3.Close\n");
        scanf("%d", &number);
        if(number == 3)
            exit(0);
        if(number == 1){
            printf("\nPlease write the name of the text file to encode in the following format: aladdin.txt\n");
            scanf("%s", text);
        } else {
            printf("\nPlease write the name of the text file to decode in the following format: encoded.txt\n");
            scanf("%s", text);
        }
        printf("\nPlease write the key code that will be utilized. i.e. format: prograavanzada\n");
        scanf("%s", key);

        createProcess(number, text, key);
    }
}