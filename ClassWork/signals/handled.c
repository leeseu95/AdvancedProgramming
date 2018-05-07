/*
    Program to test the definition of signal with sigaction
    It can take a number of seconds to live as an argument

    Gilberto Echeverria
    gilecheverria@yahoo.com
    28/02/2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define ALARM_TIME 15
#define LIFETIME 20

// Function declarations
void setupHandlers();
void waitLoop();
// Signal handlers
void onInterrupt(int signal);
void onStop(int signal);

int main(int argc, char * argv[])
{
    int seconds;

    // Use a timeout obtained as an argument
    if (argc >= 2)
        seconds = atoi(argv[1]);
    // Otherwise use a default time
    else
        seconds = LIFETIME;

    // Change the signal handlers
    setupHandlers();

    // Wait a few seconds before quiting on its own
    waitLoop(seconds);

    return 0;
}

// Function definitions

// Modify the signal handlers for specific events
void setupHandlers()
{
    struct sigaction new_action;
    struct sigaction old_action;

    // Change the action for the Ctrl-C input (SIGINT)
    new_action.sa_handler = onInterrupt;
    // Set the mask to the empty set
    if ( sigemptyset(&new_action.sa_mask) == -1 )
    {
        perror("ERROR: sigemptyset");
        exit(EXIT_FAILURE);
    }
    if ( sigaction(SIGINT, &new_action, NULL) == -1 )
    {
        perror("ERROR: sigaction");
        exit(EXIT_FAILURE);
    }

    // Change the action for the Ctrl-Z input (SIGTSTP)
    new_action.sa_handler = onStop;
    // Set the flag to restore previous behaviour after catching the signal once
    new_action.sa_flags = SA_RESETHAND;
    if ( sigaction(SIGTSTP, &new_action, &old_action) == -1 )
    {
        perror("ERROR: sigaction");
        exit(EXIT_FAILURE);
    }

    // Check what the previous disposition was for SIGTSTP
    if (old_action.sa_handler == SIG_DFL)
    {
        printf("Default behaviour was the previous disposition for SIGTSTP\n");
    }
}

// Wait a few seconds until the program ends
void waitLoop(int seconds)
{
    int i;

    // Show the PID so that it can be sent signals form the terminal
    //  or other process
    printf("My PID is: %d\n", getpid());

    printf("Will finish on my own in %3d s", seconds);
    fflush(stdout);
    // Loop to wait for something to happen
    for (i=seconds-1; i>=0; i--)
    {
        sleep(1);
        printf("\b\b\b\b\b%3d s", i);
        fflush(stdout);
    }
    printf("\nTime out. Good bye\n");
}

void onInterrupt(int signal)
{
    printf("\nCatching the Ctrl-C signal: %d\n", signal);
}

void onStop(int signal)
{
    printf("\nCatching the Ctrl-Z signal: %d for the only time\n", signal);
}
