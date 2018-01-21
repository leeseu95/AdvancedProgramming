/*
    Library for operations on matrices
    Defines a data type for a matrix of floats

    Gilberto Echeverria
    gilecheverria@yahoo.com
    04/05/2017
*/

#ifndef MATRICES_H
#define MATRICES_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Structure to keep the information of the matrices
typedef struct {
    int rows;
    int cols;
    float ** data;
} matrix_t;

///// FUNCTION DECLARATIONS /////

// Show a matrix on the screen
void printMatrix(const matrix_t * matrix);

// Create an identity matrix
void makeIdentity(matrix_t * matrix);

// Multiply 2 matrices and store the result in the third parameter
void multiplyMatrices(const matrix_t * matrix1, const matrix_t * matrix2, matrix_t * result);

// Add 2 matrices and store the result in the third parameter
void addMatrices(const matrix_t * matrix1, const matrix_t * matrix2, matrix_t * result);

// Allocate the memory necessary to store the data in the matrices
void createMatrixArrays(matrix_t * matrix);

// Store random numbers in the matrix
void fillRandomMatrix(matrix_t * matrix);

// Release the memory used by a matrix
void freeMatrix(matrix_t * matrix);

// Read a matrix from a file
int readMatrix(char * filename, matrix_t * matrix);

// Write a matrix into a file
int	writeMatrix(char * filename, const matrix_t * matrix);

#endif