//Seung Hoon Lee Kim
//A01021720
//Assignment 3 - PPM Images

#ifndef PIXELS_H
#define PIXELS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum color_enum {R, G, B} color_t;

//Structure to sore asingle pixel
typedef struct pixel_struct {
    // unsigned short red;
    // unsigned short green;
    // unsigned short blue;
    signed short int data[3];
} pixel_t;

//Structure for an image
typedef struct ppm_struct {
    char magic_number[3];
    int width;
    int height;
    int max_value;
    pixel_t ** pixels;
} ppm_t;

//Function declarations
void allocateMemory(ppm_t * image);
void readImageFile(char * filename, ppm_t * image);
void writeImageFile(char * filename, const ppm_t * image);
void getNegativeImage(ppm_t * image);
void freeMemory(ppm_t * image);
void getAsciiPixels(ppm_t * image, FILE * file_ptr);
void getBinaryPixels(ppm_t * image, FILE * file_ptr);
void rotateImageDegrees(ppm_t* image, ppm_t * tempImage, int angle);
void rotateImage(ppm_t * image, ppm_t * tempImage);
void copyImage(ppm_t * image, ppm_t * tempImage);

#endif