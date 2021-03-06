/*
    Client program to get the value of PI
    This program connects to the server using sockets

    Gilberto Echeverria
    gilecheverria@yahoo.com
    21/02/2018
*/

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


#define BUFFER_SIZE 1024

///// FUNCTION DECLARATIONS
void usage(char * program);
int openSocket(char * address, char * port);
void requestPI(int connection_fd);

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
    int chars_read;
    unsigned long int iterations;
    double result;
    // Structure to indicate the sockets to poll
    struct pollfd test_fds[1];
    int poll_result;
    int timeout = 1000; // In milliseconds (1 sec)

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
    
    while (1)
    {
        // POLL
        // Fill in the data for the structure
        test_fds[0].fd = connection_fd;
        test_fds[0].events = POLLIN;
        // Call poll
        poll_result = poll(test_fds, 1, timeout);
        
        if (poll_result == -1)
        {
            fatalError("poll");
        }
        else if (poll_result == 0)
        {
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

    // RECV
    // Receive the request
    chars_read = recv(connection_fd, buffer, BUFFER_SIZE, 0);
    if (chars_read == -1)
    {
        fatalError("recv");
    }

    sscanf(buffer, "%lf", &result);
    // Print the result
    printf("The value for PI is: %.20lf\n", result);
}
