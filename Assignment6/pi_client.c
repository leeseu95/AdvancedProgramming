//Seung Hoon Lee - A01021720
//Signal homework

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
// Library for polling functionality
#include <sys/poll.h>
// Custom libraries
#include "fatal_error.h"

//Libraries for signal homework
#include <signal.h>
#include "get_pi.h"

#define BUFFER_SIZE 1024

///// FUNCTION DECLARATIONS
void usage(char * program);
int openSocket(char * address, char * port);
void requestPI(int connection_fd);

void setupHandlers();
void onInterrupt(int signal);
void onStop(int signal);

int globalFlagClient = 0;

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int connection_fd;

    printf("\n=== CLIENT FOR COMPUTING THE VALUE OF pi ===\n");

    // Check the correct arguments
    if (argc != 3)
    {
        usage(argv[0]);
    }

    // Start the server
    connection_fd = openSocket(argv[1], argv[2]);

    setupHandlers();
	// Listen for connections from the clients
    requestPI(connection_fd);
    // Close the socket
    close(connection_fd);

    return 0;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {server_address} {port_number}\n", program);
    exit(EXIT_FAILURE);
}

/*
    Open the socket to the server
    Returns the file descriptor for the socket
    Remember to close the socket when finished
*/
int openSocket(char * address, char * port)
{
    struct addrinfo hints;
    struct addrinfo * server_info = NULL;
    int connection_fd;

    // Prepare the hints structure
    bzero (&hints, sizeof hints);
    // Set to use IPv4
    hints.ai_family = AF_INET;
    // Set type of socket
    hints.ai_socktype = SOCK_STREAM;
    // Set to look for the address automatically
    hints.ai_flags = AI_PASSIVE;

    // GETADDRINFO
    // Use the presets to get the actual information for the socket
    if (getaddrinfo(address, port, &hints, &server_info) == -1)
    {
        fatalError("getaddrinfo");
    }

    // SOCKET
    // Open the socket using the information obtained
    connection_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (connection_fd == -1)
    {
        close(connection_fd);
        fatalError("socket");
    }

    // CONNECT
    // Connect to the server
    if (connect(connection_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        fatalError("connect");
    }

    // FREEADDRINFO
    // Free the memory used for the address info
    freeaddrinfo(server_info);

    return connection_fd;
}

void requestPI(int connection_fd)
{
    char buffer[BUFFER_SIZE];
    char bufferIters[BUFFER_SIZE];
    char bufferFlagClient[BUFFER_SIZE];
    // char bufferMessage[BUFFER_SIZE];
    int chars_read;
    unsigned long int iterations;
    double result;
    unsigned long int counter;
    // Structure to indicate the sockets to poll
    struct pollfd test_fds[1];
    int poll_result;
    int timeout = 1000; // In milliseconds (1 sec)
    // char *message;
    globalFlagClient = 0;
    printf("Enter the number of iterations for PI: ");
    scanf("%lu", &iterations);

    // Prepare the request to the server
    sprintf(buffer, "%lu\n", iterations);
    
    // SEND
    // Send the response
    if (send(connection_fd, buffer, strlen(buffer) + 1, 0) == -1 )
    {
        fatalError("send");
    }
    
    // sprintf(bufferFlagClient, "%d\n", globalFlagClient);
    // // Send the response
    // if (send(connection_fd, bufferFlagClient, strlen(buffer) + 1, 0) == -1 )
    // {
    //     fatalError("send");
    // }    

    while (1)
    {
        // POLL
        // Fill in the data for the structure
        test_fds[0].fd = connection_fd;
        test_fds[0].events = POLLIN;

        // Call poll
        poll_result = poll(test_fds, 1, timeout);

        if(globalFlagClient == 1) {
                // printf("Debugging\n");
                // Prepare the request to the server
                bzero(&bufferFlagClient, BUFFER_SIZE);
                sprintf(bufferFlagClient, "%d\n", globalFlagClient);
                // // Send the response
                if (send(connection_fd, bufferFlagClient, strlen(buffer) + 1, 0) == -1 )
                {
                    fatalError("send");
                }
                // pause();
                // sleep(1);
                // // Clear the buffer
                bzero(&buffer, BUFFER_SIZE);
                bzero(&bufferIters, BUFFER_SIZE);

                chars_read = recv(connection_fd, buffer, BUFFER_SIZE, 0);
                if (chars_read == -1)
                {
                    fatalError("recv");
                }
                
                sscanf(buffer, "%lf", &result);
                sscanf(bufferIters, "%lu", &counter);

                printf("The value for PI is: %.20lf\nWith current iterations before the program exited: %lu\n\n", result, counter);
        }
        if (poll_result == -1)
        {
            fatalError("poll");
        }
        else if (poll_result == 0)
        {
            // sleep(1);
            printf("Still nothing to receive\n");
        }
        else
        {
            if (test_fds[0].revents & POLLIN)
            {
                break;
            }
        }
    }

    // Clear the buffer
    bzero(&buffer, BUFFER_SIZE);
    bzero(&bufferIters, BUFFER_SIZE);
    // bzero(&bufferMessage, BUFFER_SIZE);

    // RECV
    // Receive the request
    chars_read = recv(connection_fd, buffer, BUFFER_SIZE, 0);
    if (chars_read == -1)
    {
        fatalError("recv");
    }
    sscanf(buffer, "%lf", &result);

    // RECV
    // Receive the request
    chars_read = recv(connection_fd, bufferIters, BUFFER_SIZE, 0);
    if (chars_read == -1)
    {
        fatalError("recv");
    }
    sscanf(bufferIters, "%lu", &counter);

    // chars_read = recv(connection_fd, bufferMessage, BUFFER_SIZE, 0);
    // if (chars_read == -1)
    // {
    //     fatalError("recv");
    // }
    // sscanf(bufferMessage, "%s", message);

    // Print the result
    if(counter < iterations) {
        printf("The Server has closed unexpectedly, the current value for pi/iters will be printed\n");
    } else {
        printf("The Server has finished the task correctly, the value for pi/iters will be printed\n");
    }
    // printf("%s\n", message);
    printf("The value for PI is: %.20lf\nWith current iterations before the program exited: %lu\n\n", result, counter);
}

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

void onInterrupt(int signal)
{
    printf("\nCatching the Ctrl-C signal: %d", signal);
    printf("\nExiting program with server, current value of PI and iters will be printed\n\n");
    globalFlagClient = 1;
    // pause();
    // sleep(1);
}

void onStop(int signal)
{
    printf("\nCatching the Ctrl-Z signal: %d for the only time", signal);
    printf("\nExiting program with server, current value of PI and iters will be printed\n\n");
    globalFlagClient = 1;
}