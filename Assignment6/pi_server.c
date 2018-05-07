//Seung Hoon Lee - A01021720
//Signal homework

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
// Custom libraries
#include "fatal_error.h"

#include <ifaddrs.h>

// Library for polling functionality
#include <sys/poll.h>

//Libraries for signal homework
#include <signal.h>

int globalFlagServer = 0;

#define BUFFER_SIZE 1024
#define MAX_QUEUE 5

///// FUNCTION DECLARATIONS
void usage(char * program);
void printLocalIPs();
int initServer(char * port);
void waitForConnections(int server_fd);
void attendRequest(int client_fd);
double computePI(unsigned long int iterations);

void setupHandlers();
void onInterrupt(int signal);
void onStop(int signal);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int server_fd;

    printf("\n=== SERVER FOR COMPUTING THE VALUE OF pi ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }

	// Show the IPs assigned to this computer
	printLocalIPs();

    // Start the server
    server_fd = initServer(argv[1]);
    // Setup handlers
    setupHandlers();
	// Listen for connections from the clients
    waitForConnections(server_fd);
    // Close the socket
    close(server_fd);

    return 0;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}

/*
	Show the local IP addresses, to allow testing
	Based on code from:
		https://stackoverflow.com/questions/20800319/how-do-i-get-my-ip-address-in-c-on-linux
*/
void printLocalIPs()
{
	struct ifaddrs * addrs;
	struct ifaddrs * tmp;
	
	// Get the list of IP addresses used by this machine
	getifaddrs(&addrs);
	tmp = addrs;

    printf("Server IP addresses:\n");

	while (tmp) 
	{
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
		{
		    // Get the address structure casting as IPv4
			struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            // Print the ip address of the local machine
			printf("%s: %s\n", tmp->ifa_name, inet_ntoa(pAddr->sin_addr));
		}
		// Advance in the linked list
		tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);
}

/*
    Prepare and open the listening socket
    Returns the file descriptor for the socket
    Remember to close the socket when finished
*/
int initServer(char * port)
{
    struct addrinfo hints;
    struct addrinfo * server_info = NULL;
    int server_fd;
    int reuse = 1;

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
    if (getaddrinfo(NULL, port, &hints, &server_info) == -1) {
        fatalError("getaddrinfo");
    }

    // SOCKET
    // Open the socket using the information obtained
    server_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (server_fd == -1) {
        close(server_fd);
        fatalError("socket");
    }

    // SETSOCKOPT
    // Allow reuse of the same port even when the server does not close correctly
    if (setsockopt (server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
        fatalError("setsockopt");
    }

    // BIND
    // Connect the port with the desired port
    if (bind(server_fd, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        fatalError("bind");
    }

    // LISTEN
    // Start listening for incomming connections
    if (listen(server_fd, MAX_QUEUE) == -1) {
        fatalError("listen");
    }

    // FREEADDRINFO
    // Free the memory used for the address info
    freeaddrinfo(server_info);

    printf("Server ready\n");

    return server_fd;
}

/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    pid_t new_pid;

    // Get the size of the structure to store client information
    client_address_size = sizeof client_address;
    while (1)
    {
        // ACCEPT
        // Wait for a client connection
        client_fd = accept(server_fd, (struct sockaddr *) &client_address, &client_address_size);
        if (client_fd == -1) {
            fatalError("accept");
        }

        // Create a child to deal with the new client
        new_pid = fork();
        // Parent process
        if (new_pid > 0)
        {
            // Close the socket to the new client
            close(client_fd);
        }
        // Child process
        else if (new_pid == 0)
        {
            // Get the data from the client
            inet_ntop (client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
            printf("Received incomming connection from %s on port %d\n", client_presentation, client_address.sin_port);
            
            // Deal with the client
            attendRequest(client_fd);
            
            // Finish the child process
            close(client_fd);
            exit(EXIT_SUCCESS);
        }
        // Error
        else
        {
            fatalError("Unable to fork");
        }
    }
}

/*
    Hear the request from the client and send an answer
*/
void attendRequest(int client_fd)
{
    char buffer[BUFFER_SIZE];
    char bufferIters[BUFFER_SIZE];
    // char bufferMessage[BUFFER_SIZE];
    int chars_read;
    unsigned long int iterations;
    double result = 4;
    int sign = -1;
    unsigned long int divisor = 3;
    unsigned long int counter = 0;
    // char *message;
    struct pollfd test_fds[1];
    int poll_result;

    // Clear the buffer to avoid errors
    bzero(&buffer, sizeof buffer);
    bzero(&bufferIters, sizeof bufferIters);
    // bzero(&bufferMessage, sizeof bufferMessage);

    // RECV
    // Read the request from the client
    chars_read = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (chars_read == 0) {
        printf("Client disconnected");
        return;
    }
    if (chars_read == -1) {
        printf("Client receive error");
        return;
    }

    // Get the numerical value for iterations
    sscanf(buffer, "%lu", &iterations);

    printf(" > Got request from client with iterations=%lu\n", iterations);

    //We start the computePI iterations
    for (counter = 0; counter<iterations; counter++)
    {
        result += sign * (4.0/divisor);
        sign *= -1;
        divisor += 2;

        test_fds[0].fd = client_fd;
        test_fds[0].events = POLLIN;

        poll_result = poll(test_fds, 1, 0);
        
        if(test_fds[0].revents && POLLIN && poll_result != -1) {
            // printf("GASDGA");
            printf("Sending to client the current pi/Iterations since he's disconnecting\n");
            // printf("%lu\n", counter);
            // printf("%lf\n", result);
            sprintf(buffer, "%.20lf\n", result); //We save whatever value we have for result currently
            if (send(client_fd, buffer, strlen(buffer) + 1, 0) == -1) {
                printf("Could not send reply");
            }
            sprintf(bufferIters, "%lu\n", counter); //Save whatever iteration we're at currently
            if (send(client_fd, bufferIters, strlen(buffer) + 1, 0) == -1) {
                printf("Could not send reply");
            }
            exit(0);
        }

        if(globalFlagServer == 1) { //If the flag is activated (aka the signal interruption) from the server
            // strcpy(message, "The Server Has been Closed unexpectedly, the current value of pi/iters will be printed\n");
            sprintf(buffer, "%.20lf\n", result); //We save whatever value we have for result currently
            if (send(client_fd, buffer, strlen(buffer) + 1, 0) == -1) {
                printf("Could not send reply");
            }
            sprintf(bufferIters, "%lu\n", counter); //Save whatever iteration we're at currently
            if (send(client_fd, bufferIters, strlen(buffer) + 1, 0) == -1) {
                printf("Could not send reply");
            }
            // sprintf(bufferMessage, "%s\n", message);
            // if (send(client_fd, bufferMessage, strlen(buffer) + 1, 0) == -1) {
            //     printf("Could not send reply");
            // }
            exit(0);
        }
    }

    // printf("iters:%lu\n", iterations); //Debug
    // SEND
    // Write back the reply
    // strcpy(message, "The Server has finished the task correctly, the current value of pi/iters will be printed\n");
    sprintf(buffer, "%.20lf\n", result); //We use another buffer to save the result
    if (send(client_fd, buffer, strlen(buffer) + 1, 0) == -1) {
        printf("Could not send reply");
    }
    sprintf(bufferIters, "%lu\n", iterations);//Buffer to send to save the iterations done if it exited without problem
    if (send(client_fd, bufferIters, strlen(buffer) + 1, 0) == -1) {
        printf("Could not send reply");
    }
    // sprintf(bufferMessage, "%s\n", message);
    // if (send(client_fd, bufferMessage, strlen(buffer) + 1, 0) == -1) {
    //     printf("Could not send reply");
    // }
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
    printf("\nCatching the Ctrl-C signal: %d\n", signal);
    printf("\nClosing the server, giving the current value of pi to users\n\n");
    globalFlagServer = 1;
}

void onStop(int signal)
{
    printf("\nCatching the Ctrl-Z signal: %d for the only time\n", signal);
    printf("\nClosing the server, giving the current value of pi to users\n\n");
    globalFlagServer = 1;
}