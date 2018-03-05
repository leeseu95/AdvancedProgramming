//Black-Jack Program
//Seung Hoon Lee - A0102172
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

#define BUFFER_SIZE 1024
#define MAX_QUEUE 5

///// FUNCTION DECLARATIONS
void usage(char * program);
void printLocalIPs();
int initServer(char * port);
void waitForConnections(int server_fd);
void attendRequest(int client_fd);
void startDeck(int deckSize, int deck[deckSize]);
void printCards(int deckSize, int deck[deckSize]);
void hitCard(int deckSize, int deck[deckSize]);
int checkLose(int deckSize, int deck[deckSize]);
int totalScore(int deckSize, int deck[deckSize]);
void attendBetPrice(int client_fd);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int server_fd;

    printf("\n=== SERVER FOR SEUNGY's BLACKJACK ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }

	// Show the IPs assigned to this computer
	printLocalIPs();

    // Start the server
    server_fd = initServer(argv[1]);
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
            attendBetPrice(client_fd);
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
void attendBetPrice(int client_fd) {
    char buffer[BUFFER_SIZE];
    int chars_read;
    int totalBetPrice;

    bzero(&buffer, BUFFER_SIZE);
    
    // RECV
    // Read the request from the client
    chars_read = recv(client_fd, buffer, sizeof buffer, 0);
    if (chars_read == 0) {
        printf("Client disconnected");
        return;
    }
    if (chars_read == 0) {
        printf("Client recieve error");
        return;
    }

    //Get the numerical value for iterations
    sscanf(buffer, "%d", &totalBetPrice);

    printf(" > Got price pool total as:%d\n", totalBetPrice);

    // SEND
    // Write back the reply
    sprintf(buffer, "%d\n", totalBetPrice);
    if (send(client_fd, buffer, strlen(buffer) + 1, 0) == -1) {
        printf("Could not send reply");
    }
}

void attendRequest(int client_fd)
{
    char buffer[BUFFER_SIZE];
    int chars_read;

    //The server dealer's deck
    int dealerDeckSize = 2;
    int dealerDeck[dealerDeckSize];
    
    //Variables to paly with
    int dealerScore;
    int playerScore;

    // Clear the buffer to avoid errors
    bzero(&buffer, BUFFER_SIZE);

    // RECV
    // Read the request from the client
    chars_read = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (chars_read == 0)
    {
        printf("Client disconnected");
        return;
    }
    if (chars_read == -1)
    {
        printf("Client receive error");
        return;
    }

    startDeck(dealerDeckSize, dealerDeck);
    // printCards(dealerDeckSize, dealerDeck);
    dealerScore = totalScore(dealerDeckSize, dealerDeck);
    while ( dealerScore <= 16 ) {
        hitCard(dealerDeckSize, dealerDeck);
        dealerDeckSize += 1;
        dealerScore = totalScore(dealerDeckSize, dealerDeck);
    }

    printf("\nDealer's Cards are:\n");
    printCards(dealerDeckSize, dealerDeck);

    // Get the numerical value for iterations
    sscanf(buffer, "%d", &playerScore);

    printf(" > Got the player's score as: %d\n", playerScore);


    sprintf(buffer, "%d\n", dealerScore);
    if (send(client_fd, buffer, strlen(buffer) + 1, 0) == -1) {
        printf("Could not send reply");
    }
}

void printCards(int deckSize, int deck[deckSize]) {
    for (int i = 0; i < deckSize; i++) { //Iterate through the deckSize to initialize the deck
        printf("%d\t", deck[i]);
    }
    printf("\n\n");
}

void startDeck(int deckSize, int deck[deckSize]) {
    for (int i = 0; i < deckSize; i++) { //Iterate through the deckSize to initialize the deck
        deck[i] = (rand() % 10 + 1); //Give cards between 1 and 11
    }
}

void hitCard(int deckSize, int deck[deckSize]) {
    deck[deckSize] = (rand() % 10 + 1); //add a new card to our deck
    deckSize = deckSize + 1;
}

int checkLose(int deckSize, int deck[deckSize]) {
    int total = 0;
    for (int i = 0; i < deckSize; i++) {
        total += deck[i];
    }
    if (total > 21) 
        return 1;
    else
        return 0;
}

int totalScore(int deckSize, int deck[deckSize]) {
    int total = 0;
    for (int i = 0; i < deckSize; i++) {
        total += deck[i];
    }
    return total;
}
void printOptions() {
    printf("Please choose what you'd like to do next\n"); //Self explanatory function
    printf("1. Hit\n");
    printf("2. Stand\n");
}