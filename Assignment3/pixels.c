//Seung Hoon Lee Kim
//A01021720
//Assignment 3 - PPM Images

#include "pixels.h"

//Function Definitions
void allocateMemory(ppm_t * image) {
    //Allocate Memory
    image->pixels = malloc (image->height * sizeof(pixel_t *));
    //Allocate the Memory for all the data in the image
    image->pixels[0] = calloc (image->height * image->width, sizeof(pixel_t));

    //Add the rest of the pointers to the INDEX array
    for (int i = 1; i < image->height; i++){
        image->pixels[i] = image->pixels[0] + image->width * i;
    }
}

void freeMemory(ppm_t * image) {
    //Free the DATA array
    free (image->pixels[0]);
    //Free index array
    free (image->pixels);
}

void readImageFile(char * filename, ppm_t * image) { //Function done in class to read
    FILE * file_ptr = NULL;

    file_ptr = fopen (filename, "r");
    if(!file_ptr) {
        printf("Unable to open the file'%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    fscanf(file_ptr, "%s", image->magic_number);
    fscanf(file_ptr, "%d", &image->width);
    fscanf(file_ptr, "%d", &image->height);
    fscanf(file_ptr, "%d", &image->max_value);
    allocateMemory(image);

    if (!strncmp(image->magic_number, "P3", 3)) {
        getAsciiPixels(image, file_ptr);
    } else if (!strncmp(image->magic_number, "P6", 3)) {
        getBinaryPixels(image, file_ptr);
    } else {
        printf("Not a correct PPM format");
    }
}

void writeImageFile(char * filename, const ppm_t * image) {
    int i, j;
    FILE* file_ptr = fopen(filename, "w"); //Create our file pointer
    // Validate that the file could be opened
    if (file_ptr == NULL)
    {
        printf("ERROR: Unable to open file '%s'\n", filename);
        exit(EXIT_FAILURE); //Exit the function if it didnt work
    }

    // Write the Magic number of our image
    fprintf(file_ptr, "%s\n", image->magic_number);
    //Write the width and height of our image
    fprintf(file_ptr, "%d %d\n", image->width, image->height);
    //Write the max value of our image
    fprintf(file_ptr, "%d\n", image->max_value);

    //Start to write our matrixes of pixels
    for (i = 0; i < image->height; i++)
    {
        for (j = 0; j < image->width; j++)
        {
            fprintf(file_ptr, "%hd ", image->pixels[i][j].data[R]);
            fprintf(file_ptr, "%hd ", image->pixels[i][j].data[G]);
            fprintf(file_ptr, "%hd ", image->pixels[i][j].data[B]);
        }
        fprintf(file_ptr, "\n");
    }
    fclose(file_ptr);
}

void getAsciiPixels(ppm_t * image, FILE * file_ptr) {
    for(int r = 0; r < image->height; r++) {
        for (int c = 0; c < image->width; c++) {
            fscanf (file_ptr, "%hd", &image->pixels[r][c].data[R]);
            fscanf (file_ptr, "%hd", &image->pixels[r][c].data[G]);
            fscanf (file_ptr, "%hd", &image->pixels[r][c].data[B]);
        }
    }
}

void getBinaryPixels(ppm_t * image, FILE * file_ptr) {
    fread (image->pixels[0], sizeof(pixel_t), image->width * image->height, file_ptr);
}

void getNegativeImage(ppm_t * image) {

    for (int i = 0; i < image->height; i++) //Iterate through all of our rows
    {
        for (int j = 0; j < image->width; j++) //Iterate through all of our columns
        {
            //We use the max value to just subtract the max value of the pixel value and use the absolute value of it
            image->pixels[i][j].data[R] = abs(image->max_value - image->pixels[i][j].data[R]);
            image->pixels[i][j].data[G] = abs(image->max_value - image->pixels[i][j].data[G]);
            image->pixels[i][j].data[B] = abs(image->max_value - image->pixels[i][j].data[B]);
        }
    }
}

void rotateImageDegrees(ppm_t* image, ppm_t * tempImage, int angle) {
    if(angle % 90 != 0) { //Only accept angles that our 90 degree intervals
        printf("Error: Rotate image function only accepts intervals of 90 degrees");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < (angle/90); i++) { //Call rotateImage function as many times as needed depending on the angle
        rotateImage(image, tempImage);  //We first call our rotation function
        copyImage(image, tempImage); //Copy that rotation from temp to image
    }
}

void copyImage(ppm_t * image, ppm_t * tempImage) { //Simple function to copy what's on tempImage to image
    image->height = tempImage->height; //Set the M x N the same for both images
    image->width = tempImage->width;
    for(int i = 0; i < image->height; i++) { //Iterate through rows
        for(int j = 0; j < image->width; j++) { //iterate through cols
            image->pixels[i][j].data[R] = tempImage->pixels[i][j].data[R];
            image->pixels[i][j].data[G] = tempImage->pixels[i][j].data[G];
            image->pixels[i][j].data[B] = tempImage->pixels[i][j].data[B];
        }
    }
}

void rotateImage(ppm_t * image, ppm_t * tempImage) { //Rotate imag efunction
    int tempHeight = image->width; //Store these variables for use in our iteration
    int tempWidth = image->height;
    tempImage->height = tempHeight; //Set our temp's Height/width to a different number than original image thisi s for M x N usage
    tempImage->width = tempWidth;

    for (int i = 0; i < image->height; i ++) {
        for (int j = 0; j < image->width; j++) {
            tempImage->pixels[(tempHeight - 1) - j][i].data[R] = image->pixels[i][j].data[R]; //Rotate our matrix
            tempImage->pixels[(tempHeight - 1) - j][i].data[G] = image->pixels[i][j].data[G];
            tempImage->pixels[(tempHeight - 1) - j][i].data[B] = image->pixels[i][j].data[B];
        }
    }
}