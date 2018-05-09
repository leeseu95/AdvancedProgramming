/*
    WRITE YOUR NAME HERE:
    SEUNG HOON LEE - A01021720
    THREAD VERSION


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
void equalPGMImages(pgm_t * board_image, pgm_t * board_imageFinal);
void changeNewPGM(pgm_t * board_image, pgm_t * board_imageFinal, int interations);
void changeLife(int posX, int posY, pgm_t * board_image, pgm_t * board_imageFinal);
// ADD YOUR FUNCTION DECLARATIONS HERE

int main(int argc, char * argv[])
{
    char * start_file = "Boards/sample_4.txt";
    int iterations = 5;

    printf("\n=== GAME OF LIFE ===\n");
    printf("{By: SEUNG HOON LEE - A01021720}\n");

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
    //Initialize the image and the pgm structure
    image_t image_initTemp = {0, 0, NULL};
    pgm_t pgm_Initial = {"", 0, image_initTemp};

    //We make a new one to keep storing the original image to work on
    image_t image_finalTemp = {0, 0, NULL};
    pgm_t pgm_Final = {"", 0, image_finalTemp};
    char writeFirst[STRING_SIZE] = "0.pgm";

    //Prepare the images that we'll be modifying
    readPGMFile(start_file, &pgm_Initial);
    readPGMFile(start_file, &pgm_Final);

    //Write the first image as 0.pgm
    writePGMFile(writeFirst, &pgm_Initial);

    //Change the Image through iterations
    changeNewPGM(&pgm_Initial, &pgm_Final, iterations);

    //Free our Image
    freeImage(&image_initTemp);
    freeImage(&image_finalTemp);
}

void changeNewPGM(pgm_t * board_image, pgm_t * board_imageFinal, int iterations) {
    char write_PGM[STRING_SIZE];
    int i, j, l, k, iters;

    for(iters = 1; iters <= iterations; iters++ ){
        
        for(i = 0; i < board_image->image.width/2; i++ ) {
            for(j = 0; j < board_image->image.height; j++ ) {
                changeLife(i, j, board_image, board_imageFinal);
            }
            // printf("%d", 1);
        }
        for(l = board_image->image.width/2; l < board_image->image.width; l++ ) {
            for(k = 0; k < board_image->image.height; k++ ) {
                changeLife(l, k, board_image, board_imageFinal);
            }
            // printf("%d", 1);
        }
        snprintf(write_PGM, sizeof(write_PGM), "%d.pgm", iters);
        writePGMFile(write_PGM, board_imageFinal);
        equalPGMImages(board_image, board_imageFinal);
    }
}

void equalPGMImages(pgm_t * board_image, pgm_t * board_imageFinal) {
    for( int i = 0; i < board_image->image.width; i++ ) {
        for( int j = 0; j < board_image->image.height; j++ ) {
            board_image->image.pixels[i][j].value = board_imageFinal->image.pixels[i][j].value;
        }
    }
}

//Function to check a position of the board and check if it lives
void changeLife(int posX, int posY, pgm_t * board_image, pgm_t * board_imageFinal) {
    //Initialize positions
    int posXLeft = posX - 1;
    int posXRight = posX + 1;
    int posYUp = posY - 1;
    int posYDown = posY + 1;

    //Have a counter for the neighbours
    int neighboursAlive = 0;

    // printf("Val: %d ", board_image->image.pixels[posX][posY].value);
    //Check if either x or Y are in an edge
    //[-1, -1] Not on edge
    if(posX == 0) 
        posXLeft = board_image->image.width - 1;
    else if (posX == board_image->image.width - 1)
        posXRight = 0;
    if (posY == 0) 
        posYUp = board_image->image.height - 1;
    else if (posY == board_image->image.height - 1)
        posYDown = 0;

    //Ifs too check our neighbours
    //Left Side
    //[-1, -1]
    if(board_image->image.pixels[posXLeft][posYUp].value == 1)
        neighboursAlive += 1;
    //[-1, 0]
    if(board_image->image.pixels[posXLeft][posY].value == 1)
        neighboursAlive += 1;
    //[-1, 1]
    if(board_image->image.pixels[posXLeft][posYDown].value == 1)
        neighboursAlive += 1;
    
    //Center
    //[0, -1]
    if(board_image->image.pixels[posX][posYUp].value == 1)
        neighboursAlive += 1;
    //[0, +1]
    if(board_image->image.pixels[posX][posYDown].value == 1)
        neighboursAlive += 1;

    //Right
    //[1, -1]
    if(board_image->image.pixels[posXRight][posYUp].value == 1)
        neighboursAlive += 1;
    //[1, 0]
    if(board_image->image.pixels[posXRight][posY].value == 1)
        neighboursAlive += 1;    
    //[1, -1]
    if(board_image->image.pixels[posXRight][posYDown].value == 1)
        neighboursAlive += 1;

    //Check conditions
    //If less than two neighbours, it dies
    if( board_image->image.pixels[posX][posY].value == 1 && neighboursAlive < 2 )
        board_imageFinal->image.pixels[posX][posY].value = 0;
    //If two or three neighbours, lives
    else if(board_image->image.pixels[posX][posY].value == 1 && (neighboursAlive == 2 || neighboursAlive == 3)) 
        board_imageFinal->image.pixels[posX][posY].value = 1;
    //If more than three, it dies
    else if(board_image->image.pixels[posX][posY].value == 1 && neighboursAlive > 3)
        board_imageFinal->image.pixels[posX][posY].value = 0;
    //If 3 neighbours and the cell is dead, it revives
    else if(board_image->image.pixels[posX][posY].value == 0 && neighboursAlive == 3)
        board_imageFinal->image.pixels[posX][posY].value = 1;
}
