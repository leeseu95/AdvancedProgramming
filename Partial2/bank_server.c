/*
Partial 2 - Seung Lee A01021720
Bank System
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Signals library
#include <errno.h>
#include <signal.h>
// Sockets libraries
#include <netdb.h>
#include <sys/poll.h>
// Posix threads library
#include <pthread.h>

// Custom libraries
#include "bank_codes.h"
#include "sockets.h"
#include "fatal_error.h"

#define NUM_ACCOUNTS 4
#define BUFFER_SIZE 1024
#define MAX_QUEUE 5

///// Structure definitions

// Data for a single bank account
typedef struct account_struct {
    int id;
    float balance;
} account_t;

// Data for the bank operations
typedef struct bank_struct {
    // Store the total number of operations performed
    int total_transactions;
    // An array of the accounts
    account_t * account_array;
} bank_t;

// Structure for the mutexes to keep the data consistent
typedef struct locks_struct {
    // Mutex for the number of transactions variable
    pthread_mutex_t transactions_mutex;
    // Mutex array for the operations on the accounts
    pthread_mutex_t * account_mutex;
} locks_t;

// Data that will be sent to each structure
typedef struct data_struct {
    // The file descriptor for the socket
    int connection_fd;
    // A pointer to a bank data structure
    bank_t * bank_data;
    // A pointer to a locks structure
    locks_t * data_locks;
} thread_data_t;



///// FUNCTION DECLARATIONS
void usage(char * program);
void setupHandlers();
void initBank(bank_t * bank_data, locks_t * data_locks);
void waitForConnections(int server_fd, bank_t * bank_data, locks_t * data_locks);
void * attentionThread(void * arg);
int checkValidAccount(int account);
void closeBank(bank_t * bank_data, locks_t * data_locks);
/*
    EXAM: Add your function declarations here
*/
//Signal handlers
void onInterrupt(int signal);
void onStop(int signal);
int globalFlagServer = 0;


///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int server_fd;
    bank_t bank_data;
    locks_t data_locks;

    printf("\n=== SEUNGY'S BANK SERVER ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }

    // Configure the handler to catch SIGINT
    setupHandlers();

    // Initialize the data structures
    initBank(&bank_data, &data_locks);

	// Show the IPs assigned to this computer
	printLocalIPs();
    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
	// Listen for connections from the clients
    waitForConnections(server_fd, &bank_data, &data_locks);
    // Close the socket
    close(server_fd);

    // Clean the memory used
    closeBank(&bank_data, &data_locks);

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
void initBank(bank_t * bank_data, locks_t * data_locks)
{
    // Set the number of transactions
    bank_data->total_transactions = 0;

    // Allocate the arrays in the structures
    bank_data->account_array = malloc(NUM_ACCOUNTS * sizeof (account_t));
    // Allocate the arrays for the mutexes
    data_locks->account_mutex = malloc(NUM_ACCOUNTS * sizeof (pthread_mutex_t));

    // Initialize the mutexes, using a different method for dynamically created ones
    //data_locks->transactions_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&data_locks->transactions_mutex, NULL);
    for (int i=0; i<NUM_ACCOUNTS; i++)
    {
        //data_locks->account_mutex[i] = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&data_locks->account_mutex[i], NULL);
    }
}

/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd, bank_t * bank_data, locks_t * data_locks)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    pthread_t new_tid;
    thread_data_t * connection_data = NULL;
    int poll_response;
	int timeout = 500;		// Time in milliseconds (0.5 seconds)

    // Get the size of the structure to store client information
    client_address_size = sizeof client_address;

    while (1)
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
            printf("Total transactions done: %d\n", connection_data->bank_data->total_transactions);
            fatalError("ERROR: poll");
        }
		// Timeout finished without reading anything
        else if (poll_response == 0)
        {
            // printf("No response after %d milliseconds\n", timeout);
        }
		// There is something ready at the socket
        else
        {
            // Check the type of event detected
            if (test_fds[0].revents & POLLIN)
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
                connection_data = malloc(sizeof(thread_data_t)); //Malloc must be used first before filling it up with information
                connection_data->connection_fd = client_fd;
                connection_data->bank_data = bank_data;
                connection_data->data_locks = data_locks;

				// CREATE A THREAD
                // We create the thread by giving the new_tid, the function that we'll be doing and our connection data
                pthread_create(&new_tid, NULL, attentionThread, (void *)connection_data);
            }
        }
    }

    // Print the total number of transactions performed
    // We print the total transactions done after the server has been closed
    // printf("Total threads done: %d", connection_data->bank_data->total_transactions);
}

/*
    Hear the request from the client and send an answer
*/
void * attentionThread(void * arg)
{
    //Connection Variables
    int connection_fd;
    thread_data_t* connection_data;
    account_t* account_array;
    //Buffer where we'll store our information
    char buffer[BUFFER_SIZE];
    //Operations
    int operation;
    int account;
    float amount;
    int status;
    float amountInCase = 0;

    // Receive the data for the bank, mutexes and socket file descriptor
    // We declare a new conection data to access it
    connection_data = (thread_data_t*) arg;
    connection_fd = connection_data->connection_fd;
    account_array = connection_data->bank_data->account_array;
    
    // Extract mutexes
    pthread_mutex_t transactions_mutex = connection_data->data_locks->transactions_mutex;
    pthread_mutex_t* accounts_mutex = connection_data->data_locks->account_mutex;

    // Loop to listen for messages from the client
    while(1) {
        // Receive the request
        // Clear the buffer to avoid errors
        bzero(&buffer, sizeof buffer);

        //RECV   
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Client closed the connection\n");
            break;
        }  

        // Recieve the information inside the buffer
        sscanf(buffer, "%d %d %f", &operation, &account, &amount);
        
        printf("Request Test operation: %d, account: %d, amount: %f", operation, account, amount);

        pthread_mutex_lock(&transactions_mutex);
        pthread_mutex_lock(accounts_mutex); //Lock the account mutexes;
        if(checkValidAccount(account)) {
            switch(operation) { //We start a switch to see what the operation will do
            // Check balance
            case 0:
                status = 0; //The status would always be 0 when checking balance
                sprintf(buffer, "%d %f", status, account_array[account].balance); //We send the current balance of the account (buffer)
                printf("\nTotal balance of account %d is %f: ", account, account_array[account].balance);
                printf("\n");
                sendString(connection_fd, buffer); //Send it 
                break;
            // Deposit into account
            case 1:
                status = 0;
                account_array[account].balance += amount; //We add the amount to the balance of the account specified
                sprintf(buffer, "%d %f", status, account_array[account].balance); //Prepare the buffer
                printf("\nThe money has been deposited correctly\nTotal balance is: %f of account #%d", account_array[account].balance, account);
                printf("\n");
                sendString(connection_fd, buffer); //Send them oney
                break;
            // Withdraw from account
            case 2:
                if(account_array[account].balance < amount) { //If the balance is less than the amount specified
                    printf("Insufficient funds");
                    status = 1; //Insufficient amount
                    sprintf(buffer, "%d %f", status, account_array[account].balance); //Prepare the buffer
                    sendString(connection_fd, buffer); //Send them oney
                    break;
                } else { //Sufficient amounts
                    account_array[account].balance -= amount;
                    status = 0;
                    sprintf(buffer, "%d %f", status, account_array[account].balance); //Prepare the buffer
                    printf("\nThe money has been withdrawn correctly\nTotal balance is: %f of account #%d", account_array[account].balance, account);
                    printf("\n");
                    sendString(connection_fd, buffer); //Send them oney
                    break;
                }
            //Case of exiting from the server
            case 3:
                status = 3;
                sprintf(buffer, "%d %f", status, account_array[account].balance);
                sendString(connection_fd, buffer); //Send them oney
                break;
            }
            // Process the request being careful of data consistency
            // account_array[account].balance += amount;

            // Update the number of transactions
            connection_data->bank_data->total_transactions ++;

            // Send a reply
            // SEND
            // sprintf(buffer, "%d %f", status, account_array->balance ); //We use the buffer to save the result before sending it
            // sendString(connection_fd, buffer);
        } else {
            status = 2;
            sprintf(buffer, "%d %f", status, amountInCase); //Prepare the buffer
            sendString(connection_fd, buffer); //Send the response
            printf("Wrong account number #%d, please enter a correct account #\n", account);
        }
        pthread_mutex_unlock(accounts_mutex); //Unlock it after everything has been done
        pthread_mutex_unlock(&transactions_mutex);
    }
    pthread_exit(NULL);
}

/*
    Free all the memory used for the bank data
*/
void closeBank(bank_t * bank_data, locks_t * data_locks)
{
    printf("DEBUG: Clearing the memory for the thread\n");
    free(bank_data->account_array);
    free(data_locks->account_mutex);
}

/*
    Return true if the account provided is within the valid range,
    return false otherwise
*/
int checkValidAccount(int account)
{
    return (account >= 0 && account < NUM_ACCOUNTS);
}

void onInterrupt(int signal)
{
    printf("\nCatching the Ctrl-C signal: %d\n", signal);
    printf("\nClosing the server, letting all users know the server is closing\n\n");
    globalFlagServer = 1;
}

void onStop(int signal)
{
    printf("\nCatching the Ctrl-Z signal: %d for the only time\n", signal);
    printf("\nClosing the server, letting all users know the server is closing\n\n");
    globalFlagServer = 1;
}