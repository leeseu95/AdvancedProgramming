/*
    WRITE YOUR NAME HERE:



    Simulation of Conway's Game of Life using OpenMP
    Based on the explanations at:
        https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
    Online versions:
        http://pmav.eu/stuff/javascript-game-of-life-v3.1.1/
        https://bitstorm.org/gameoflife/

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pgm_image.h"
// ADD YOUR EXTRA LIBRARIES HERE

#define STRING_SIZE 50

///// Function declarations /////
void usage(const char * program);
void lifeSimulation(int iterations, char * start_file);
void preparePGMImage(const void * board, pgm_t * board_image);
void saveAsPGM(const void * board, pgm_t * board_image, int iteration);
// ADD YOUR FUNCTION DECLARATIONS HERE

int main(int argc, char * argv[])
{
    char * start_file = "Boards/sample_4.txt";
    int iterations = 5;

    printf("\n=== GAME OF LIFE ===\n");
    printf("{By: WRITE YOUR NAME HERE}\n");

    // Parsing the arguments
    if (argc == 2)
    {
        iterations = atoi(argv[1]);
    }
    else if (argc == 3)
    {
        iterations = atoi(argv[1]);
        start_file = argv[2];
    }
    else if (argc != 1)
    {
        usage(argv[0]);
        return 1;
    }

    // Run the simulation with the iterations specified
    printf("Running the simulation with file '%s' using %d iterations\n", start_file, iterations);
    lifeSimulation(iterations, start_file);

    return 0;
}

// Print usage information for the program
void usage(const char * program)
{
    printf("Usage:\n");
    printf("%s [iterations] [board file]\n", program);
}

// Main loop for the simulation
void lifeSimulation(int iterations, char * start_file)
{

}

// Get the memory necessary to store an image
void preparePGMImage(const void * board, pgm_t * board_image)
{

}

// Save the contents of the board as a PGM image
void saveAsPGM(const void * board, pgm_t * board_image, int iteration)
{

}
