/*
Final Project
Seung Hoon Lee - A01021720
Samuel Torrero - A01361063
*/

// The different types of operations available
//Operations available at first for cards
typedef enum valid_operations {CHECK, HIT, STAND, EXIT} operation_t;

//Operations for betting
typedef enum valid_bets {SETTLE, DOBULEDOWN, WITHDRAW} betting_t;

// The types of responses available
typedef enum valid_responses {OK, INSUFFICIENT, NO_ACCOUNT, BYE, ERROR} response_t;
