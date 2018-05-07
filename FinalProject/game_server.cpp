/*
Final Project
Seung Hoon Lee - A01021720
Samuel Torrero - A01361063
BlackJack Game Integrating SFML - server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <string>
#include <iostream>
#include <sstream>
// Signals library
#include <errno.h>
#include <signal.h>
// Sockets libraries
#include <netdb.h>
#include <sys/poll.h>
// Posix threads library
#include <pthread.h>

// Custom libraries
#include "game_codes.h"
#include "sockets.h"
#include "fatal_error.h"
#include <time.h>

//Max Number of players in a game will be 5
#define NUM_PLAYERS 4
#define BUFFER_SIZE 1024
#define MAX_QUEUE 5

//Number of Players Counter
int playerCounter = 0;
int gameWinPlayerCounter = 0;
int players = 0;
float prizePool = 0;
float prizePoolReset = 0;
int dealerScore;
int scores[NUM_PLAYERS];

using namespace std;

///// Structure definitions

// Data for a single bank account
typedef struct account_struct {
    int id;
    int scoreCard;
    int done;
    float balance;
} account_t;

// Data for the bank operations
typedef struct game_struct {
    // Store the total number of operations performed
    int total_bets;
    int computerScoreCard;
    // An array of the accounts
    account_t * player_array;
} game_t;

// Structure for the mutexes to keep the data consistent
typedef struct locks_struct {
    // Mutex for the number of transactions variable
    pthread_mutex_t bets_mutex;
    // Mutex array for the operations on the accounts
    pthread_mutex_t * player_mutex;
} locks_t;

// Data that will be sent to each structure
typedef struct data_struct {
    // The file descriptor for the socket
    int connection_fd;
    // A pointer to a bank data structure
    game_t * server_data;
    // A pointer to a locks structure
    locks_t * data_locks;
} thread_data_t;

int dealerDeck[2];
int dealerDeckSize = 2;

///// FUNCTION DECLARATIONS
void usage(char * program);
void setupHandlers();
void initGameServer(game_t * server_data, locks_t * data_locks);
void waitForConnections(int server_fd, game_t * server_data, locks_t * data_locks);
void * attentionThread(void * arg);
int checkValidAccount(int account);
void closeGameServer(game_t * server_data, locks_t * data_locks);
int checkWinner(int scores[], int players);
//Signal handlers
void onInterrupt(int signal);
void onStop(int signal);
int globalFlagServer = 0;
//Deck functions
void startDeck(int deckSize, int deck[]);
void printCards(int deckSize, int deck[]);
void hitCard(int deckSize, int deck[]);
int checkLose(int deckSize, int deck[]);
int totalScore(int deckSize, int deck[]);


///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int server_fd;
    game_t server_data;
    locks_t data_locks;
    
    printf("\n=== SEUNGY'S AND SAM'S BLACKJACK SERVER ===\n");
    srand(time(0));
    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }

    // Configure the handler to catch SIGINT
    setupHandlers();

    // Initialize the data structures
    initGameServer(&server_data, &data_locks);

	// Show the IPs assigned to this computer
	printLocalIPs();
    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
	// Listen for connections from the clients
    waitForConnections(server_fd, &server_data, &data_locks);
    // Close the socket
    close(server_fd);

    // Clean the memory used
    closeGameServer(&server_data, &data_locks);

    // Finish the main thread
    pthread_exit(NULL);

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
    Modify the signal handlers for specific events
*/
void setupHandlers()
{
    struct sigaction new_action;
    struct sigaction old_action;

    // Change the action for the Ctrl-C input (SIGINT)
    new_action.sa_handler = onInterrupt;
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



/*
    Function to initialize all the information necessary
    This will allocate memory for the accounts, and for the mutexes
*/
void initGameServer(game_t * server_data, locks_t * data_locks)
{
    // Set the number of transactions
    server_data->total_bets = 0;

    // Allocate the arrays in the structures
    server_data->player_array = (account_t *) malloc(NUM_PLAYERS * sizeof (account_t));
    // Allocate the arrays for the mutexes
    data_locks->player_mutex = (pthread_mutex_t *) malloc(NUM_PLAYERS * sizeof (pthread_mutex_t));

    // Initialize the mutexes, using a different method for dynamically created ones
    //data_locks->bets_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&data_locks->bets_mutex, NULL);
    for (int i=0; i<NUM_PLAYERS; i++)
    {
        //data_locks->player_mutex[i] = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&data_locks->player_mutex[i], NULL);
    }
}

/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd, game_t * server_data, locks_t * data_locks)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    pthread_t new_tid;
    thread_data_t * connection_data = NULL;
    int poll_response;
	int timeout = 1000;		// Time in milliseconds (0.5 seconds)
    int counterTimeout = 10;

    startDeck(dealerDeckSize, dealerDeck);
    // printCards(dealerDeckSize, dealerDeck);
    dealerScore = totalScore(dealerDeckSize, dealerDeck);
    while ( dealerScore <= 16 ) {
        hitCard(dealerDeckSize, dealerDeck);
        dealerDeckSize += 1;
        dealerScore = totalScore(dealerDeckSize, dealerDeck);
    }
    if (dealerScore > 21) {
        dealerScore = 0;
    }
    // Get the size of the structure to store client information
    client_address_size = sizeof client_address;

        while (counterTimeout != 0 && playerCounter < 5)
        {
            //// POLL
            // Create a structure array to hold the file descriptors to poll
            struct pollfd test_fds[1];
            // Fill in the structure
            test_fds[0].fd = server_fd;
            test_fds[0].events = POLLIN;    // Check for incomming data
            // Check if there is any incomming communication
            poll_response = poll(test_fds, 1, timeout);

            // Error when polling
            if (poll_response == -1 && globalFlagServer == 1) //If the signal is called we close the server and print the amount of threads done
            {
                // We print the total transactions done after the server has been closed
                printf("The server will be closing now");
                fatalError("ERROR: poll");
            }
            // Timeout finished without reading anything
            else if (poll_response == 0)
            {
                printf("Counter Timeout:%d\n", counterTimeout);
                // printf("No response after %d milliseconds\n", timeout);
            }
            // There is something ready at the socket
            else
            {
                // Check the type of event detected and playerCounter is less than 5
                if (test_fds[0].revents & POLLIN && playerCounter < 5)
                {
                    // ACCEPT
                    // Wait for a client connection
                    client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_size);
                    if (client_fd == -1)
                    {
                        fatalError("ERROR: accet");
                    }
                    
                    // Get the data from the client
                    inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
                    printf("Received incomming connection from %s on port %d\n", client_presentation, client_address.sin_port);

                    // Prepare the structure to send to the thread
                    // We need to access our struct of connection data and fill it up with thep arameters of thei nformation
                    connection_data = (thread_data_t *) malloc(sizeof(thread_data_t)); //Malloc must be used first before filling it up with information
                    connection_data->connection_fd = client_fd;
                    connection_data->server_data = server_data;
                    connection_data->data_locks = data_locks;

                    // CREATE A THREAD
                    // We create the thread by giving the new_tid, the function that we'll be doing and our connection data
                    pthread_create(&new_tid, NULL, attentionThread, (void *)connection_data);
                }
            }
            counterTimeout --;
            players = playerCounter;
            cout << "Current # of players: " << players << endl;
        }
}

/*
    Hear the request from the client and send an answer
*/
void * attentionThread(void * arg)
{
    //Connection Variables
    int connection_fd;
    thread_data_t* connection_data;
    account_t* player_array;
    //Buffer where we'll store our information
    char buffer[BUFFER_SIZE];
    //Operations
    int operation;
    float amount;
    int player;
    int choiceOptionDone = 0;
    int betOptionDone = 0;

    player = playerCounter;
    playerCounter ++;
    gameWinPlayerCounter ++;
    // printf("player counter:%d\n", playerCounter);
    // Receive the data for the bank, mutexes and socket file descriptor
    // We declare a new conection data to access it
    connection_data = (thread_data_t*) arg;
    connection_fd = connection_data->connection_fd;
    player_array = connection_data->server_data->player_array;
    
    // Extract mutexes
    pthread_mutex_t bets_mutex = connection_data->data_locks->bets_mutex;
    pthread_mutex_t* accounts_mutex = connection_data->data_locks->player_mutex;

    //Send #1-------------------------------------------------------------
    //Send the player number to the player that just connected in
    player_array[player].balance = 5000;

    while(1) {
        choiceOptionDone = 0;
        betOptionDone = 0;
        sprintf(buffer, "%d %f", player, player_array[player].balance);
        sendString(connection_fd, buffer);
        //RECIEVE  #1-----------------------------------------------------------
        // Clear the buffer to avoid errors
        bzero(&buffer, sizeof buffer);
        //Recieve the string to see what the prize pool is
        //RECV   
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Client closed the connection\n");
        }  
        // Recieve the information inside the buffer
        sscanf(buffer, "%f", &amount);
        //---Lock-----------------------------------
        pthread_mutex_lock(&bets_mutex);
        pthread_mutex_lock(accounts_mutex); //Lock the player mutexes;
        //Deduct from the player's balance
        player_array[player].balance -= amount;

        // We add the prizepool
        prizePool += amount;
        printf("Current Prize Pool %f:\n", prizePool);
        //SEND #2-----------------------------------------------
        //Send the player number to the player that just connected in the prize pool
        sprintf(buffer, "%f %f", prizePool, player_array[player].balance);
        sendString(connection_fd, buffer);

        player_array[player].done = 0;
        pthread_mutex_unlock(accounts_mutex); //Unlock it after everything has been done
        pthread_mutex_unlock(&bets_mutex);

        // ----------------------------------------------------
        //RECIEVE #2----------------------------------------------------
        // Receive the request to update prize pool for all clients
        // Clear the buffer to avoid errors
        bzero(&buffer, sizeof buffer);
        //RECV   
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Client closed the connection\n");
        }  
        // Recieve the information inside the buffer
        sscanf(buffer, "%d", &choiceOptionDone);
        if (choiceOptionDone == 1) {
            // Prepare the message to the server
            sprintf(buffer, "%f %d", prizePool, players);
            // SEND #3---------------------------------------
            // Send the request
            sendString(connection_fd, buffer);
        }
        
        //----------------------------------------------------------------------------------------------------
        //-----------------------------------------------------------------------------------------------------
        //END OF PRIZEPOOL AND INITIALIZATION ------------------------------------------------------------------
        //UPDATE PRIZEPOOL AFTER PHASE 2
        //RECIEVE #3----------------------------------------------------------------------------
        // Receive the request to update prize pool for all clients
        // Clear the buffer to avoid errors
        bzero(&buffer, sizeof buffer);
        //RECV   
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Client closed the connection\n");
        }
        // Recieve the information inside the buffer
        sscanf(buffer, "%d %d %f %d", &betOptionDone, &player, &amount, &operation);

        //DEAL WITH THE OPERATION BET REQUEST---------------------------------------
        if (betOptionDone == 1) {
            switch(operation) {
                case SETTLE:
                    playerCounter --;
                    break;
                case DOBULEDOWN:
                    playerCounter --;
                    pthread_mutex_lock(&bets_mutex);
                    pthread_mutex_lock(accounts_mutex); //Lock the player mutexes;
                    player_array[player].balance -= amount;
                    cout << "account:" << player << "\t" << player_array[player].balance << endl;
                    prizePool += amount;
                    pthread_mutex_unlock(accounts_mutex); //Unlock it after everything has been done
                    pthread_mutex_unlock(&bets_mutex);
                    break;
                case WITHDRAW:
                    playerCounter --;
                    pthread_mutex_lock(&bets_mutex);
                    pthread_mutex_lock(accounts_mutex); //Lock the player mutexes;
                    player_array[player].balance += amount/2;
                    pthread_mutex_unlock(accounts_mutex); //Unlock it after everything has been done
                    pthread_mutex_unlock(&bets_mutex);
                    cout << "account:" << player << "\t" << player_array[player].balance << endl;
                    prizePool -= amount/2;
                    break;
            }
        }

        while(playerCounter != 0) {
            cout << "Waiting for all players to finish their bet decisions" << endl;
            sleep(1);
        }

        // Prepare the message to the server
        sprintf(buffer, "%f", prizePool);
        // SEND #4-------------------------------
        // Send the request
        sendString(connection_fd, buffer);
        //--------------------------------------------------------------------------------
        //-------------------------------------------------------------------------------
        //RECIEVE #4----------------------------------------------------------------------------
        // Receive the request to update score card
        int score = 0;
        // Clear the buffer to avoid errors
        bzero(&buffer, sizeof buffer);
        //RECV   
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Client closed the connection\n");
        }  
        // Recieve the information inside the buffer
        sscanf(buffer, "%d %d", &score, &player);
        gameWinPlayerCounter --;
        player_array[player].scoreCard = score;

        while(gameWinPlayerCounter != 0 ) {
            //Loop to wait until all players are done
        }

        int winnerScore;
        //Insert the player's score into an array
        for (int i = 0; i <= players; i++) {
            scores[i] = player_array[i].scoreCard;
            if(winnerScore < player_array[i].scoreCard) {
                winnerScore = player_array[i].scoreCard;
            }
        }
        
        int winnerNum = checkWinner(scores, players);
        int playerWinner = winnerNum;
        if (dealerScore > player_array[playerWinner].scoreCard) {
            winnerNum = 4;
            winnerScore = dealerScore;
        }
        // Prepare the message to the server
        sprintf(buffer, "%d %d", winnerNum, winnerScore);
        // SEND #5------------------------------------------
        // Send the request OF THE WINNER PLAYER to the player
        sendString(connection_fd, buffer);
        
        if (dealerScore > player_array[playerWinner].scoreCard) {  //If the dealer is the winner
            cout << "The winner of the game is the computer with cards: " << endl;
            printCards(dealerDeckSize, dealerDeck);
        } else {
            cout << "The winner of the game is player # " << winnerNum << " with a score of : " << winnerScore << endl;
            printCards(dealerDeckSize, dealerDeck);
            //Add the prizepool money to the winner's balance
            player_array[playerWinner].balance += prizePool / players;
        }
        
        cout << "Dealer Score: " << dealerScore << endl;
        
        for(int i = 0; i < 5; i++) {
            sleep(1);
        }
        
        //Wait to reset all the settings of the server
        playerCounter = players;
        gameWinPlayerCounter = players;
        prizePool = 0;
        dealerDeckSize = 2;
        startDeck(dealerDeckSize, dealerDeck);
        // printCards(dealerDeckSize, dealerDeck);
        dealerScore = totalScore(dealerDeckSize, dealerDeck);
        while ( dealerScore <= 16 ) {
            hitCard(dealerDeckSize, dealerDeck);
            dealerDeckSize += 1;
            dealerScore = totalScore(dealerDeckSize, dealerDeck);
        }

        // cout << "Dealer cards: " << endl;
        // printCards(dealerDeckSize, dealerDeck);

        if (dealerScore > 21) {
            dealerScore = 0;
        }
    }
    pthread_exit(NULL);
}

/*
    Free all the memory used for the bank data
*/
void closeGameServer(game_t * server_data, locks_t * data_locks)
{
    printf("DEBUG: Clearing the memory for the thread\n");
    free(server_data->player_array);
    free(data_locks->player_mutex);
}

/*
    Return true if the account provided is within the valid range,
    return false otherwise
*/
int checkValidAccount(int account)
{
    return (account >= 0 && account < NUM_PLAYERS);
}

void onInterrupt(int signal)
{
    printf("\nCatching the Ctrl-C signal: %d\n", signal);
    printf("\nClosing the server, letting all users know the server is closing\n\n");
    globalFlagServer = 1;
    exit(0);
}

void onStop(int signal)
{
    printf("\nCatching the Ctrl-Z signal: %d for the only time\n", signal);
    printf("\nClosing the server, letting all users know the server is closing\n\n");
    globalFlagServer = 1;
}

int checkWinner(int scores[], int players) {
    int max = 0;
    int winner = 0;
    for (int i = 0; i < players + 1; i ++) {
        if(scores[i] > max) {
            max = scores[i];
            winner = i;
        }
    }
    return winner;
}

//Functions for the computer's cards
void printCards(int deckSize, int deck[]) {
    for (int i = 0; i < deckSize; i++) { //Iterate through the  to initialize the deck
        printf("%d\t", deck[i]);
    }
    printf("\n\n");
}

void startDeck(int deckSize, int deck[]) {
    for (int i = 0; i < deckSize; i++) { //Iterate through the deckSize to initialize the deck
        deck[i] = (rand() % 10 + 1); //Give cards between 1 and 11
    }
}

void hitCard(int deckSize, int deck[]) {
    deck[deckSize] = (rand() % 10 + 1); //add a new card to our deck
    deckSize = deckSize + 1;
}

int checkLose(int deckSize, int deck[]) {
    int total = 0;
    for (int i = 0; i < deckSize; i++) {
        total += deck[i];
    }
    if (total > 21) 
        return 1;
    else
        return 0;
}

int totalScore(int deckSize, int deck[]) {
    int total = 0;
    for (int i = 0; i < deckSize; i++) {
        total += deck[i];
    }
    return total;
}