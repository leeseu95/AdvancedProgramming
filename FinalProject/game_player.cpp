/*
Final Project
Seung Hoon Lee - A01021720
Samuel Torrero - A01361063
BlackJack Game Integrating SFML - player
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <time.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
// Custom libraries
#include "game_codes.h"
#include "sockets.h"
#include "fatal_error.h"
// Posix threads library
#include <pthread.h>
// // SFML Libraries
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <array>

#define BUFFER_SIZE 1024

using namespace std;

///// FUNCTION DECLARATIONS
void usage(char * program);
void blackjackOperations(int connection_fd);
void startDeck(int deckSize, int deck[]);
void printCards(int deckSize, int deck[]);
void hitCard(int deckSize, int deck[]);
int checkLose(int deckSize, int deck[]);
int totalScore(int deckSize, int deck[]);
void *run_sfml(void *arg);

//Blackjack Variables
char buffer[BUFFER_SIZE];
int playerNum;
float amount;
float balance;
float prizePool; // TOTAL BET
char option = 'c';
char option2 = 'c';
int status;
int betDone = 0;
int choiceOptionDone = 0;
int betOptionDone = 0;
int lostGame = 0;
int buttonCounter = 0;
int playerDeckSize = 2;
int playerDeck[5];
int score = 0; //SCORE DEL JUGADOR
int winnerNum = 0; // WINNER DEL JUEGO
int winnerScore = 0; //SCORE DEL GANADOR
int numOfPlayers = 0; //NUMBER OF PLAYERS

int gameEnd = 0; //End of game flag
// Variables to use in graphic environment
int playerNumSFML = 0;
int winnerNumSFML = 0;
operation_t operation;
betting_t operation2;

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int connection_fd;

    printf("\n=== WELCOME TO SEUNGY'S AND SAM'S BLACKJACK ===\n");

    // Check the correct arguments
    if (argc != 3)
    {
        usage(argv[0]);
    }
    pthread_t new_tid;
    srand(time(0)); //Use the current time as seed generator
    // Start the server
    connection_fd = connectSocket(argv[1], argv[2]);
    // CREATE A THREAD
    // We create the thread by giving the new_tid, the function that we'll be doing and our connection data
    pthread_create(&new_tid, NULL, run_sfml, (void *)NULL);
	// Use the bank operations available
    blackjackOperations(connection_fd);
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
    Main menu with the options available to the user
*/
void blackjackOperations(int connection_fd)
{   
    amount = 0;
    
    //End OF PRIZE POOL AND INITIALIZATION ---------------------------------------------------------
    while (1)
    {
        //amount -> option -> option2 ->
        //Restart the variables for each round
        option = 'c';
        option2 = 'c';
        betDone = 0;
        choiceOptionDone = 0;
        betOptionDone = 0;
        amount = 0;
        buttonCounter = 0;
        score = 0;
        winnerNum = 0;
        playerNum = 0;
        playerDeckSize = 2;
        
        // Clear the buffer to avoid errors
        bzero(&buffer, sizeof buffer);
        //RECIEVE #1----------------------------------------
        // RECV
        // Receive the response
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Server closed the connection\n");
        }
        // Extract the data
        sscanf(buffer, "%d %f", &playerNum, &balance);
        printf("Please choose your starting bet amount, your current balance is: %f\n", balance);
        
        gameEnd = 0; // Flag to stop drawing winner text
        // Reset player and winner graphic variables
        playerNumSFML = 0;
        winnerNumSFML = 0;
        
        // scanf("%f", &amount);
        // while(amount < balance && amount < 0) {
        //     cout << amount << endl;
        //     printf("Invalid amount of bet, please choose an amount within your balance\n");
        // }

        //AMOUNT NECESITA SER 0 PARA QUE SE QUEDE INFINITO
        // while(amount == 0 || amount > balance) {
        //     sleep(0);
        //     if (amount > balance) {
        //         printf("Wrong amount, please enter a number within your balance\n");
        //     }
        // }

        printf("The game is starting with all remaining players, please wait a second\n");
        sleep(3);

        // Prepare the message to the server
        sprintf(buffer, "%f", amount);
        // SEND #1-----------------------------------------------
        // Send the request
        sendString(connection_fd, buffer);

        // Clear the buffer to avoid errors
        bzero(&buffer, sizeof buffer);
        //RECIEVE #2------------------------------------------------------
        // RECV
        // Receive the response of prize pool
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Server closed the connection\n");
        }
        // Extract the data
        sscanf(buffer, "%f %f", &prizePool, &balance);
        printf("Current prize pool is: %f\n", prizePool);

        startDeck(playerDeckSize, playerDeck);
        printCards(playerDeckSize, playerDeck);
        score = totalScore(playerDeckSize, playerDeck);
        
        printf("Game Menu:\n");
        printf("\th. Hit\n");
        printf("\ts. Stand with my current cards\n");
        printf("Select an option: ");
        //SFML
        while(option != 's') {
            switch(option)
            {
                //Hit
                case 'h':
                    printf("\nHitting a card, your new cards are:\n ");
                    hitCard(playerDeckSize, playerDeck);
                    playerDeckSize += 1;
                    printCards(playerDeckSize, playerDeck);
                    operation = HIT;
                    sleep(2);
                    score = totalScore(playerDeckSize, playerDeck);
                    if(checkLose(playerDeckSize, playerDeck) == 1) {
                        printf("You've lost the game automatically after getting more than 21, your money will be taken\nThank you for playing ^-^\nWaiting for the game to finish\n");
                        option = 's';
                        betDone = 1;
                        option2 = 's';
                        operation2 = SETTLE;
                        lostGame = 1;
                        amount = 0;
                    }
                    else
                        sleep(1);
                    break;
                // Stand
                case 's':
                    operation = STAND;
                    if(lostGame == 0) {
                        printf("\nYou will stand with your current cards, your cards are:\n");
                        printCards(playerDeckSize, playerDeck);
                    }
                    break;
                //Exit
                case 'x':
                    printf("\nThanks for using the program. Bye!\n");
                    operation = EXIT;
                    exit(0);
                    break;
                // Incorrect option
                default:
                    continue;
            }
        }
        //Recieve the updated prize pool
        choiceOptionDone = 1;
        // Prepare the message to the server
        sprintf(buffer, "%d", choiceOptionDone);
        // SEND #2 -------------------------------------------
        // Send the request
        sendString(connection_fd, buffer);
        // Clear the buffer to avoid errors
        bzero(&buffer, sizeof buffer);
        //RECIEVE #3------------------------------------------
        // RECV
        // Receive the response of prize pool
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Server closed the connection\n");
        }
        // Extract the data
        sscanf(buffer, "%f %d", &prizePool, &numOfPlayers);
        printf("\n\n\nCurrent UPDATED prize pool is: %f\n\n", prizePool);
        cout << "PLAYERS: " << numOfPlayers << endl << "Prize Pool :" << prizePool << endl << "Score: " << score << endl;

        printf("Your Current Cards are: \n");
        printCards(playerDeckSize, playerDeck);
        printf("Amount you bet: %f\n\n", amount);
        printf("Current balance: %f\n\n", balance);
        printf("Bet Menu:\n");
        printf("\ts. Settle with current bet\n");
        printf("\td. Double Down\n");
        printf("\tw. Withdraw (half of your money will be lost)\n");
        printf("Select an option: ");

        //SFML
        while(betDone == 0) {
            // scanf(" %c", &option2);

            switch(option2) {
            case 's':
                printf("\n\nSettling down with the current amount of bet\nTotal Balance: %f\n", balance);
                operation2 = SETTLE;
                amount = 0;
                betDone = 1;
                break;
            case 'd':
                if(balance >= amount){
                    operation2 = DOBULEDOWN;
                    balance -= amount;
                    printf("\n\nDouble down of the bet\nTotal Balance Left: %f\n", balance);
                    betDone = 1;
                    break;
                } else {
                    printf("\n\nNot enough for a double down, please choose a different option\n");
                    break;
                }
            case 'w':
                printf("\n\nLeaving the game, your money will be lost\nThank you for playing\n");
                betDone = 1;
                operation2 = WITHDRAW;
                balance += balance/2;
                printf("Current balance : %f\n", balance);
                break;
            default:
                // printf("Invalid option. Try again ...\n");
                // Skip the rest of the code in the while
                continue;
            }
        }

        //UPDATE THE PHASE 2 BETTING (LAST UPDATE)
        //Recieve the updated prize pool
        betOptionDone = 1;
        // Prepare the message to the server
        sprintf(buffer, "%d %d %f %d", betOptionDone, playerNum, amount, operation2);
        // SEND #3-----------------------------------------------
        // Send the request
        sendString(connection_fd, buffer);
        
        // Clear the buffer to avoid errors
        bzero(&buffer, sizeof buffer);
        // RECV
        //RECIEVE #4------------------------------------------------
        // Receive the response of prize pool
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Server closed the connection\n");
        }
        // Extract the data
        sscanf(buffer, "%f", &prizePool);
        printf("\n\n\nCurrent LAST UPDATE on prize pool is: %f\n\n", prizePool);
        //------------------------------------------------------------------
        //-------------------------------------------------------------------
        //PHASE 2 BETTING DONE ----------------------------------------------
        //-------------------------------------------------------------------

        // Count total score of the player
        // Prepare the message to the server
        if(lostGame == 0) {
            score = totalScore(playerDeckSize, playerDeck);
        } else if (lostGame == 1) {
            score = 0;
        }
        sprintf(buffer, "%d %d", score, playerNum);
        // SEND #4-----------------------------------------------
        // Send the request
        sendString(connection_fd, buffer);

        bzero(&buffer, sizeof buffer);
        //RECIEVE #5 -----------------------------------------------
        // Receive the response of player number
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Server closed the connection\n");
        }
        // Extract the data
        sscanf(buffer, "%d %d", &winnerNum, &winnerScore);

        if(playerNum == winnerNum) {
            printf("\nWINNER WINNER CHICKEN DINNER\nTotal wins: %f\n\n\n", prizePool);
            playerNumSFML = playerNum;
            winnerNumSFML = winnerNum;
            gameEnd = 1;
        } else if (winnerNum == 4) {
            printf("\nThe winner of this game is the dealer! With a score of %d\nRestarting game now\n\n\n", winnerScore);
            playerNumSFML = playerNum;
            winnerNumSFML = winnerNum;
            gameEnd = 1;
        } else if (playerNum != winnerNum ) {
            printf("\nWinner of this game is player #%d with a score of %d\nRestarting game now\n\n\n", winnerNum, winnerScore);
            playerNumSFML = playerNum;
            winnerNumSFML = winnerNum;
            gameEnd = 1;
        }
    }
}

void printCards(int deckSize, int deck[]) {
    // printf("Your current cards are: \n");
    for (int i = 0; i < deckSize; i++) { //Iterate through the deckSize to initialize the deck
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
    // cout << deckSize << endl;
    deck[deckSize] = (rand() % 10 + 1); //add a new card to our deck
    // cout << "card:" << rand() % 10 + 1 << endl;
    // cout << deck[deckSize] << endl;
    deckSize += 1;
}

int checkLose(int deckSize, int deck[]) {
    int total = 0; //Have a variable to store the total amount of score
    for (int i = 0; i < deckSize; i++) { 
        total += deck[i];
    }
    if (total > 21)  //Check if it's an automatic loss
        return 1;
    else
        return 0;
}

int totalScore(int deckSize, int deck[]) { //Function to get the total score of the deck
    int total = 0;
    for (int i = 0; i < deckSize; i++) {
        total += deck[i];
    }
    return total;
}

void *run_sfml(void *arg)
{
    //SFML Variables
    int counter = 0;
    int bet = 0;
    int figure_random = 0;
    int number_random = 0;
    int amounttemp = 0;
    int hasMadeFirstBet = 0;
    int hasStanded = 0;
    string input_bet;

    // Window size
    sf::RenderWindow window(sf::VideoMode(1024, 622), "Custom Blackjack Game", sf::Style::Default);

    // Background image
    sf::Texture background;
    sf::Sprite backgroundImage;
    if (!background.loadFromFile("images/poker-table.png"))
        std::cout << "Error: Could not display background image" << std::endl;

    backgroundImage.setTexture(background);
    backgroundImage.setScale(0.75,0.75);

    // Set font
    sf::Font font;
    if (!font.loadFromFile("fonts/Raleway-Regular.ttf"))
        std::cout << "Can't find the font file" << std::endl;

    // Cards
    sf::Sprite cards [52];
    sf::Texture cardImage [52];
    for(int i = 0; i < 53 - 1; i++) {
        // Generate random numbers for cards
        figure_random = rand() % 4 + 1;
        number_random = rand() % 13 + 1;

        // Load card images randomly
        if (!cardImage[i].loadFromFile("images/name_test/" + to_string(figure_random) + "_" + to_string(number_random) + ".png"))
            std::cout << "Can't find the image" << std::endl;
		
        cards[i].setTexture(cardImage[i]);
		cards[i].setOrigin(cardImage[i].getSize().x/2, cardImage[i].getSize().y/2);
		cards[i].setPosition(sf::Vector2f(250 + (i*60), 320));
		cards[i].setScale(0.5,0.5);
	}

    // Back
    sf::Sprite backCard;
    sf::Texture backCardImage;
    if (!backCardImage.loadFromFile("images/new_cards/back.png"))
        std::cout << "Can't find the image" << std::endl;
    backCard.setTexture(backCardImage);
    backCard.setOrigin(backCardImage.getSize().x/2, backCardImage.getSize().y/2);
    backCard.setPosition(510.0f, 170.0f);
    backCard.setScale(0.12,0.12);

    // Button Bet
    // Bet
    sf::Texture betButton;
    sf::Sprite betButtonImage;
    if (!betButton.loadFromFile("images/buttons/bet_button.png"))
        std::cout << "Can't find the image" << std::endl;
    betButtonImage.setPosition(750.0f, 300.0f);
    betButtonImage.setTexture(betButton);
    betButtonImage.setScale(0.2,0.2);

    // Buttons Cards
    // Stand
    sf::Texture standButton;
    sf::Sprite standButtonImage;
    if (!standButton.loadFromFile("images/buttons/stand_button.png"))
        std::cout << "Can't find the image" << std::endl;
    standButtonImage.setPosition(741.0f, 350.0f);
    standButtonImage.setTexture(standButton);
    standButtonImage.setScale(0.2,0.2);

    // Hit
    sf::Texture hitButton;
    sf::Sprite hitButtonImage;
    if (!hitButton.loadFromFile("images/buttons/hit_button.png"))
        std::cout << "Can't find the image" << std::endl;
    hitButtonImage.setPosition(750.0f, 250.0f);
    hitButtonImage.setTexture(hitButton);
    hitButtonImage.setScale(0.2,0.2);

    // Buttons Betting
    // Double
    sf::Texture doubleButton;
    sf::Sprite doubleButtonImage;
    if (!doubleButton.loadFromFile("images/buttons/double_button.png"))
        std::cout << "Can't find the image" << std::endl;
    doubleButtonImage.setPosition(744.0f, 400.0f);
    doubleButtonImage.setTexture(doubleButton);
    doubleButtonImage.setScale(0.2,0.2);

    // Withdraw
    sf::Texture withdrawButton;
    sf::Sprite withdrawButtonImage;
    if (!withdrawButton.loadFromFile("images/buttons/withdraw_button.png"))
        std::cout << "Can't find the image" << std::endl;
    withdrawButtonImage.setPosition(750.0f, 200.0f);
    withdrawButtonImage.setTexture(withdrawButton);
    withdrawButtonImage.setScale(0.2,0.2);

    // Stats
    // Score
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setStyle(sf::Text::Bold);
    scoreText.setString("Score: " + to_string(score));
    scoreText.setColor(sf::Color::White);
    scoreText.setCharacterSize(25);
    scoreText.setPosition(20.0f, 0.0f);

    // Number of players
    sf::Text playersText;
    playersText.setFont(font);
    playersText.setString("Number of players: " + to_string(numOfPlayers));
    playersText.setColor(sf::Color::White);
    playersText.setCharacterSize(20);
    playersText.setPosition(270.0f, 400.0f);

    // Total bet
    sf::Text betText;
    betText.setFont(font);
    betText.setString("Total bet: " + to_string(prizePool));
    betText.setColor(sf::Color::White);
    betText.setCharacterSize(20);
    betText.setPosition(270.0f, 430.0f);

    // Winner message
    sf::Text winnerText;
    winnerText.setFont(font);
    winnerText.setStyle(sf::Text::Bold);
    winnerText.setString("Winner is: " + to_string(winnerNum));
    winnerText.setColor(sf::Color::White);
    winnerText.setCharacterSize(35);
    winnerText.setPosition(300.0f, 460.0f);

    // Bet input
    sf::String playerInput;
    sf::Text inputText;
    inputText.setFont(font);
    inputText.setColor(sf::Color::Black);
    inputText.setCharacterSize(20);
    inputText.setPosition(510.0f,310.0f);

    // Bet input instructions
    sf::Text instructionsText;
    instructionsText.setFont(font);
    instructionsText.setString("Bet must be under $5000");
    instructionsText.setColor(sf::Color::White);
    instructionsText.setCharacterSize(20);
    instructionsText.setPosition(385.0f, 360.0f);

    sf::RectangleShape textBox;
    textBox.setSize(sf::Vector2f(300, 50));
    textBox.setFillColor(sf::Color::White);
    textBox.setPosition(360.0f,300.0f);

    while (window.isOpen())
    {
        sf::Event Event;
        while (window.pollEvent(Event))
        {
            switch (Event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    exit(0);
                    break;
                case sf::Event::MouseMoved:
                {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window); // Get mouse position
                    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
                    // Change color when hover over buttons
                    // Stand button
                    if (standButtonImage.getGlobalBounds().contains(mousePosF))
                    {
                        standButtonImage.setColor(sf::Color(250, 20, 20));
                    }
                    else
                    {
                        standButtonImage.setColor(sf::Color(255, 255, 255));
                    }
                    // Bet button
                    if (betButtonImage.getGlobalBounds().contains(mousePosF))
                    {
                        betButtonImage.setColor(sf::Color(250, 20, 20));
                    }
                    else
                    {
                        betButtonImage.setColor(sf::Color(255, 255, 255));
                    }
                    // Hit button
                    if (hitButtonImage.getGlobalBounds().contains(mousePosF))
                    {
                        hitButtonImage.setColor(sf::Color(250, 20, 20));
                    }
                    else
                    {
                        hitButtonImage.setColor(sf::Color(255, 255, 255));
                    }
                    // Double button
                    if (doubleButtonImage.getGlobalBounds().contains(mousePosF))
                    {
                        doubleButtonImage.setColor(sf::Color(250, 20, 20));
                    }
                    else
                    {
                        doubleButtonImage.setColor(sf::Color(255, 255, 255));
                    }
                    // Double button
                    if (withdrawButtonImage.getGlobalBounds().contains(mousePosF))
                    {
                        withdrawButtonImage.setColor(sf::Color(250, 20, 20));
                    }
                    else
                    {
                        withdrawButtonImage.setColor(sf::Color(255, 255, 255));
                    }
                }
                break;
                case sf::Event::MouseButtonPressed: // Check for clicks in different buttons
                {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window); // Get mouse position
                    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
                    if (standButtonImage.getGlobalBounds().contains(mousePosF)) // Stand button
                    {
                        std::cout << "Clicked stand!" << std::endl;
                        hasStanded = 1;
                        if(buttonCounter == 0) {
                            option = 's'; // Stand when you get the cards
                            buttonCounter ++;
                        } else if (buttonCounter == 1) {
                            option2 = 's'; // Stand when betting
                            buttonCounter ++;
                        } else {

                        }
                    }
                    else if (betButtonImage.getGlobalBounds().contains(mousePosF)) // Bet button
                    {
                        std::cout << "Clicked bet!" << std::endl;
                        bet = 1;
                        inputText.setString(""); // Reset string
                    }
                    else if (hitButtonImage.getGlobalBounds().contains(mousePosF)) // Hit button
                    {
                        std::cout << "Clicked hit!" << std::endl;
                        option = 'h';

                        // Do nothing while there is no server response for the image
                        while (playerDeck[counter] == 0) {
                            sleep(1);
                            cout << "Waiting for server response..." << endl;
                        }
                        
                        sleep(1); // Wait to deliver cards
                        option = 'c'; // Reset option for server so it doesn't keeps hitting

                        // Keep waiting for the server
                        cout << "Getting card..." << endl;
                        sleep(1);

                        // Load card image
                        if (!cardImage[counter].loadFromFile("images/name_test/" + to_string(figure_random) + "_" + to_string(playerDeck[counter]) + ".png")) {
                            std::cout << "Can't find the image" << std::endl;
                        } else {
                            cout << "Getting card: " << to_string(playerDeck[counter]) << endl;
                            figure_random = rand() % 4 + 1; // Set random card figure for the next card
                            cards[counter].setTexture(cardImage[counter]); // Set the texture to the card
                            counter++; // Increment the counter for the next card
                        }
                    }
                    else if (doubleButtonImage.getGlobalBounds().contains(mousePosF)) // Double button
                    {
                        std::cout << "Clicked double!" << std::endl;
                        option2 = 'd';
                    }
                    else if (withdrawButtonImage.getGlobalBounds().contains(mousePosF)) // Withdraw button
                    {
                        std::cout << "Clicked withdraw!" << std::endl;
                        option2 = 'w';
                    }
                }
                break;
                case sf::Event::TextEntered:
                {
                    if (Event.text.unicode == '\b' && playerInput.getSize() > 0) { // Handle backspace
                        playerInput.erase(playerInput.getSize() - 1, 1);
                    } else if (Event.text.unicode == 13) { // Handle enter
                        bet = 0;
                        hasMadeFirstBet = 1; // Flag to stop showing bet button and show the next ones
                        stringstream geek(input_bet); // Convert string to float
                        geek >> amounttemp;
                        amount = (float)amounttemp; // Assign input value to amount
                        // cout << amount << endl;
                        input_bet = ""; // Reset string
                        playerInput = ""; // Reset string
                        sleep(4); // Wait for server response

                        // Load first card image
                        if (!cardImage[0].loadFromFile("images/name_test/" + to_string(figure_random) + "_" + to_string(playerDeck[0]) + ".png")) {
                            std::cout << "Can't find the image" << std::endl;
                        } else {
                            counter ++;
                            figure_random = rand() % 4 + 1; // Get random card figure for future use
                        }
                        cards[0].setTexture(cardImage[0]); // Set card image

                        // Load second card image
                        if (!cardImage[1].loadFromFile("images/name_test/" + to_string(figure_random) + "_" + to_string(playerDeck[1]) + ".png")) {
                            std::cout << "Can't find the image" << std::endl;
                        } else {
                            counter ++;
                        }
                        cards[1].setTexture(cardImage[1]); // Set card image
                    } else if(Event.text.unicode >= 48 && Event.text.unicode <= 57 && bet == 1){ // Input only numbers
                        playerInput += static_cast<char>(Event.text.unicode); // Cast the unicode input to a readable string
                        input_bet += static_cast<char>(Event.text.unicode); // Cast the unicode input to a readable string
                    };
                    inputText.setString(playerInput);
                }
            }
        }

        // Clear window before displaying everything
        window.clear();
        // Background
        window.draw(backgroundImage);
        // Draw back card at top
        window.draw(backCard);
        // Buttons
        // Player has made a bet, he can hit or stand with his cards
        if (hasMadeFirstBet != 0 && hasStanded == 0) {
            window.draw(standButtonImage);
            window.draw(hitButtonImage);
        }
        // Game is starting, player can only make bets
        if (hasMadeFirstBet == 0 && gameEnd == 0) {
            window.draw(betButtonImage);
        }
        // Hitting and standing has ended, player can only double, settle or withdraw his betting
        if (hasStanded != 0) {
            window.draw(standButtonImage);
            window.draw(doubleButtonImage);
            window.draw(withdrawButtonImage);
        }
    
        // Text
        window.draw(scoreText);
        scoreText.setString("Score: " + to_string(score));
        window.draw(playersText);
        playersText.setString("Number of players: " + to_string(numOfPlayers));
        window.draw(betText);
        betText.setString("Total bet: $" + to_string(prizePool));
        // Cards
        if(counter >= 0) {
            for (int i = 0; i < counter; i++) {
                window.draw(cards[i]);
            }
        }
        
        // If the flag is on then the input for betting shows
        if(bet){
            window.draw(textBox);
            window.draw(inputText);
            window.draw(instructionsText);
        }

        // Draw the winning text
        if(gameEnd == 1) {
            counter = 0; // Reset cards
            // Flags to rest buttons
            hasStanded = 0;
            hasMadeFirstBet = 0;
            // Draw the text
            window.draw(winnerText);
            // This player has won
            if (winnerNumSFML == playerNumSFML) {
                winnerText.setString("Congratulations! You win!");    
            } else if (winnerNumSFML == 4) { // Player number 4 is dealer, he won
                winnerText.setString("The winner is the dealer!");    
            } else { // Display which player won if any of the past options is true
                winnerText.setString("The winner is player number " + to_string(winnerNumSFML));
            }
        }

        // Make everything visible
        window.display();
    }
}