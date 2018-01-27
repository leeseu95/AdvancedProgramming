//Seung Hoon Lee Kim
//A01021720
//Assignment 3 - PPM Images
#define _CRT_SECURE_NO_DEPRECATE

#include "pixels.h"

int main(int argc, char **argv) {
    ppm_t ppm_A = {"", 0, 0, 0, NULL};
    ppm_t ppm_temp = {"", 0, 0, 0, NULL};

    char * fileA = "pixels.ppm"; //We use two files 1 which is the original image
    char * fileB = "pixelsWrite.ppm"; //The other one which is the one we'll be altering all the time

    if (argc == 2) { //If the file name was provided, use that instead
        fileA = argv[1];
        fileB = argv[2];
    }

    readImageFile(fileA, &ppm_A); //Initialize both of our ppm structs
    readImageFile(fileA, &ppm_temp);
    getNegativeImage(&ppm_A); //Commented for debugging, uncomment to make it work
    rotateImageDegrees(&ppm_A, &ppm_temp, 90); //Accepts only angles of 90 degrees intervals, commented for debugging 
    writeImageFile(fileB, &ppm_A);
    freeMemory(&ppm_A);  //Free both of our ppms
    freeMemory(&ppm_temp);
}