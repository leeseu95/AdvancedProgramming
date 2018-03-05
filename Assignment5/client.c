//Black-Jack Program
//Seung Hoon Lee - A0102172

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
// Custom libraries
#include "fatal_error.h"

#include <ifaddrs.h>

#define BUFFER_SIZE 1024

///// FUNCTION DECLARATIONS
void usage(char * program);
int openSocket(char * address, char * port);
void gameStart(int connection_fd);
void startDeck(int deckSize, int deck[deckSize]);
void printCards(int deckSize, int deck[deckSize]);
void hitCard(int deckSize, int deck[deckSize]);
int checkLose(int deckSize, int deck[deckSize]);
int totalScore(int deckSize, int deck[deckSize]);
void printOptions();

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int connection_fd;

    printf("\nSeungy's BlackJack is Open, WELCOME\n");

    // Check the correct arguments
    if (argc != 3)
    {
        usage(argv[0]);
    }

    // Start the server
    connection_fd = openSocket(argv[1], argv[2]);
	// Listen for connections from the clients
    gameStart(connection_fd);
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
    if (getaddrinfo(address, port, &hints, &server_info) == -1) {
        fatalError("getaddrinfo");
    }

    // SOCKET
    // Open the socket using the information obtained
    connection_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (connection_fd == -1) {
        close(connection_fd);
        fatalError("socket");
    }

    // CONNECT
    // Connect to the server
    if (connect(connection_fd, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        fatalError("connect");
    }

    // FREEADDRINFO
    // Free the memory used for the address info
    freeaddrinfo(server_info);

    return connection_fd;
}

void gameStart(int connection_fd) //The player will decide what move the server is making or if it's the player (dealer vs player)
{
    char buffer[BUFFER_SIZE];
    int chars_read;
    int playerDeckSize = 2;
    int playerDeck[playerDeckSize]; //Define our variables (decks for 2 players and their scores)
    int betPrice; //Bettings from the player
    int totalBetPrice;
    // int totalBetPrice;
    int playerChoice;
    int playerScore; //Player Score
    int dealerScore; //Dealer's score
    int blackjack = 0; //False for now
    
    srand(time(0)); //Use the current time as seed generator

    printf("-----------Welcome to Seungy's BlackJack---------\n");
    printf("Please place your bet between 2$ and 500$:\n"); //We ask the player for the bet they'd like to do
    scanf("%d", &betPrice); 
    printf("\n");
    while(betPrice < 2 || betPrice > 500) {
        printf("Please re-enter your betting price, it has to be between 2 and 500\n");
        scanf("%d", &betPrice);
        printf("\n");
    }

    // Prepare the request to the server
    sprintf(buffer, "%d\n", betPrice);
    
    // SEND
    // Send the response
    if (send(connection_fd, buffer, strlen(buffer) + 1, 0) == -1 )
    {
        fatalError("send");
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

    sscanf(buffer, "%d", &totalBetPrice);
    // Print the result
    printf("The value for the total price pool is: %d\n", totalBetPrice);



    //Start the actual game--------------------------
    printf("\nYour current cards are:\n"); //Initialize our player's cards

    startDeck(playerDeckSize, playerDeck); //Show the player's hand to the player himself
    printCards(playerDeckSize, playerDeck);

    printOptions();
    scanf("%d", &playerChoice); //Give our player the choice to hit or stnad
    while(playerChoice < 1 && playerChoice > 2) {
        printf("Please re-enter your choice, it has to be Hit or Stand");
        scanf("%d", &playerChoice);
        printf("\n");
    }

    while(playerChoice == 1) { //If our player decides to hit a card
        hitCard(playerDeckSize, playerDeck);
        playerDeckSize += 1;
        printf("Your current cards are:\n");
        printCards(playerDeckSize, playerDeck);
        if (checkLose(playerDeckSize, playerDeck) == 1) { //We check to see if the player score has gone over 21
            printf("You have lost the game for going over 21, the dealer has gotten all your money\n");
            exit(0);
        } else {
            printOptions();
            scanf("%d", &playerChoice); //Give our player the choice to hit or stnad
            while(playerChoice < 1 && playerChoice > 2) {
                printf("Please re-enter your choice, it has to be Hit or Stand");
                scanf("%d", &playerChoice);
                printf("\n");
            }
        }
    }

    if((playerDeck[0] == 1 && playerDeck[1] == 10) || (playerDeck[0] == 10 && playerDeck[1] == 1)) {
        playerScore = 21;
        blackjack = 1;
    } // In case of a blackjack
    else {
        playerScore = totalScore(playerDeckSize, playerDeck);
    }
    
    // Prepare the response to the server
    sprintf(buffer, "%d\n", playerScore);

    // SEND
    // Send the response
    if (send(connection_fd, buffer, strlen(buffer) + 1, 0) == -1) {
        fatalError("send");
    }

    // Clear the buffer
    bzero(&buffer, BUFFER_SIZE);

    // RECV
    // Receive the request
    chars_read = recv(connection_fd, buffer, BUFFER_SIZE, 0);
    if (chars_read == -1) {
        fatalError("recv");
    }

    sscanf(buffer, "%d", &dealerScore);

    // Print the result
    if ( dealerScore > 21) {
        if( playerScore == 21 && blackjack == 1) {
            printf("\nYou have won through blackjack\n\nDealer's score: %d\nPlayer's score: %d\n\n", dealerScore, playerScore);
            printf("Total winnings: %f", betPrice * 2.5);
            printf("$\n");
        } else {
            printf("\nThe dealer has lost since his score is over 21\n\nDealer's score: %d\nPlayer's score: %d\n\n", dealerScore, playerScore);
        }  
    } else {
        if (dealerScore > playerScore) {
            printf("\nThe dealer has won all the money since his score is higher than yours\n\nDealer's score: %d\nPlayer's score: %d\n\n", dealerScore, playerScore);
            printf("Total winnings: -%d", betPrice);
            printf("$\n");
        }
        else if (dealerScore == playerScore) {
            printf("\nThe dealer and you have the same score, so your bet is returned\n\nDealer's score: %d\nPlayer's score: %d\n\n", dealerScore, playerScore);
            printf("No winnings nor losses\n");
        }
        else if (playerScore > dealerScore) {
            printf("\nYou have won due to a higher score, you get your winnings and double of that\n\nDealer's score: %d\nPlayer's score: %d\n\n", dealerScore, playerScore);
            printf("Total winnings: %d", betPrice * 2);
            printf("$\n");
        }
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
    int total = 0; //Have a variable to store the total amount of score
    for (int i = 0; i < deckSize; i++) { 
        total += deck[i];
    }
    if (total > 21)  //Check if it's an automatic loss
        return 1;
    else
        return 0;
}

int totalScore(int deckSize, int deck[deckSize]) { //Function to get the total score of the deck
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