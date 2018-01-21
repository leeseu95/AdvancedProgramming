/*
    Library for operations on matrices
    Defines a data type for a matrix of floats

    Gilberto Echeverria
    gilecheverria@yahoo.com
    04/05/2017
*/

// Allow the use of 'scanf' in Visual Studio
// http://www.cplusplus.com/forum/beginner/2300/
#define _CRT_SECURE_NO_DEPRECATE

#include "matrices.h"
#include "matrices.h"

///// MAIN FUNCTION /////

int main(int argc, char * argv[])
{
    // The variables for the matrices, initialized to be empty
	matrix_t matrix_A = {0, 0, NULL};
	matrix_t matrix_B = {0, 0, NULL};
	matrix_t matrix_C = {0, 0, NULL};
    // Default names for the matrix files to use
	char * fileA = "Matrix_A.txt";
	char * fileB = "Matrix_B.txt";
	char * fileC = "Matrix_C.txt";

    // If the names of the files were provides, use those instead
    if (argc == 3)
    {
        fileA = argv[1];
        fileB = argv[2];
    }

	// Read the matrices from files:
	printf("Matrix A:\n");
	readMatrix(fileA, &matrix_A);
	printMatrix(&matrix_A);

	printf("Matrix B:\n");
	readMatrix(fileB, &matrix_B);
	printMatrix(&matrix_B);

	// Compute the multiplication
	printf("Multiplication:\n");
	multiplyMatrices(&matrix_A, &matrix_B, &matrix_C);
	printMatrix(&matrix_C);
    // Write the resulting matrix into a new file
	writeMatrix(fileC, &matrix_C);

    // Release the memory used by the matrices
    freeMatrix(&matrix_A);
    freeMatrix(&matrix_B);
    // Release the memory for the result only if it was created
    if (matrix_C.data)
    {
        freeMatrix(&matrix_C);
    }

	return 0;
}
