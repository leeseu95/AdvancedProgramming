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

///// FUNCTION DEFINITIONS /////

/*
    Show a matrix on the screen
*/
void printMatrix(const matrix_t * matrix)
{
    int i, j;

    for (i = 0; i < matrix->rows; i++)
    {
        for (j = 0; j < matrix->cols; j++)
        {
            printf("%6.2f\t", matrix->data[i][j]);
        }
        printf("\n");
    }
}

/*
    Create an identity matrix
*/
void makeIdentity(matrix_t * matrix)
{
    int i, j;

    for (i = 0; i < matrix->rows; i++)
    {
        for (j = 0; j < matrix->cols; j++)
        {
            if (i == j)
            {
                matrix->data[i][j] = 1;
            }
            else
            {
                matrix->data[i][j] = 0;
            }
        }
    }
}

/*
    Multiply 2 matrices and store the result in the third parameter
*/
void multiplyMatrices(const matrix_t * matrix_1, const matrix_t * matrix_2, matrix_t * matrix_result)
{
    int i, j, k;

    // Validate that the matrices have matching sizes and can be multiplied
    if (matrix_1->cols == matrix_2->rows)
    {
        // Set the size of the new matrix
        matrix_result->rows = matrix_1->rows;
        matrix_result->cols = matrix_2->cols;
        // Allocate the size of the array data
        // Since 'matrix_result' is already a pointer, there is no need to send the address
        createMatrixArrays(matrix_result);
        // Loops for the multiplication
        for (i = 0; i < matrix_1->rows; i++)
        {
            for (j = 0; j < matrix_2->cols; j++)
            {
                for (k = 0; k < matrix_1->cols; k++)
                {
                    matrix_result->data[i][j] += matrix_1->data[i][k] * matrix_2->data[k][j];
                }
            }
        }
    }
    // Otherwise the multiplication can not be done
    else
    {
        printf("The sizes of the matrices do not match for multiplication\n");
    }
}

/*
    Add 2 matrices and store the result in the third parameter
*/
void addMatrices(const matrix_t * matrix_1, const matrix_t * matrix_2, matrix_t * matrix_result)
{
    int i, j;

    // Validate that the two matrices have the same sizes
    if (matrix_1->cols == matrix_2->cols && matrix_1->rows == matrix_2->rows)
    {
        // Set the size of the new matrix
        matrix_result->rows = matrix_1->rows;
        matrix_result->cols = matrix_1->cols;
        // Allocate the size of the array data
        // Since 'matrix_result' is already a pointer, there is no need to send the address
        createMatrixArrays(matrix_result);
        // Loops for the addition
        for (i = 0; i < matrix_1->rows; i++)
        {
            for (j = 0; j < matrix_1->cols; j++)
            {
                matrix_result->data[i][j] = matrix_1->data[i][j] + matrix_2->data[i][j];
            }
        }
    }
    // Otherwise the matrices can not be added
    else
    {
        printf("The sizes of the matrices do not match for addition\n");
    }
}

/*
   Allocate the memory necessary to store the data in the matrices
*/
void createMatrixArrays(matrix_t * matrix)
{
    int i;

    // Allocate memory for the pointers to the rows
    matrix->data = (float **) malloc(matrix->rows * sizeof (float *));
	// Validate that the memory was assigned
    if (matrix->data == NULL)
    {
        printf("Error: Could not allocate memory for the matrix rows!\n");
        exit(EXIT_FAILURE);
    }

    // Allocate the memory for the whole matrix.
    // This will make the memory contiguous.
    // Use calloc to initialize to zeros.
    matrix->data[0] = (float *) calloc(matrix->rows * matrix->cols, sizeof (float));
	// Validate that the memory was assigned
    if (matrix->data[0] == NULL)
    {
        printf("Error: Could not allocate memory for the matrix!\n");
        exit(EXIT_FAILURE);
    }

    // Assign the pointers to the rows
    for (i=0; i<matrix->rows; i++)
    {
        matrix->data[i] = matrix->data[0] + matrix->cols * i;
    }
}

/*
   Store random numbers in the matrix
*/
void fillRandomMatrix(matrix_t * matrix)
{
    int i, j;

    // Initialize the random seed
    srand( (unsigned int) time(NULL) );
    
    // Allocate memory for the matrix
    createMatrixArrays(matrix);

    for (i=0; i<matrix->rows; i++)
    {
        for (j=0; j<matrix->cols; j++)
        {
            // Generate random float numbers between 0 and 100
            matrix->data[i][j] = rand() * 1.0 / (float) RAND_MAX;
        }
    }
}

/*
    Release the memory used by a matrix
*/
void freeMatrix(matrix_t * matrix)
{
    // Free the memory where the data is stored
    free( matrix->data[0] );
    // Free the array for the row pointers
    free(matrix->data);
    // Set the sizes to zero
    matrix->data = NULL;
    matrix->rows = 0;
    matrix->cols = 0;
}

/*
    Read a matrix from a file
    Return 1 if the file could not be opened, and 0 if the file was read correctly
*/
int readMatrix(char* filename, matrix_t * matrix)
{
    int i, j;
    FILE* file_ptr = fopen(filename, "r");
    // Validate that the file could be opened
    if (file_ptr == NULL)
    {
        printf("ERROR: Unable to open file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    // Read the sizes of the matrices. First the rows, then the columns
    fscanf(file_ptr, "%d", &matrix->rows);
    fscanf(file_ptr, "%d", &matrix->cols);
    // Allocate memory for the matrices
    createMatrixArrays(matrix);
    // Loops to read the data in the matrices
    for (i = 0; i < matrix->rows; i++)
    {
        for (j = 0; j < matrix->cols; j++)
        {
            fscanf(file_ptr, "%f", &matrix->data[i][j]);
        }
    }

    fclose(file_ptr);

    return 0;
}

/*
    Write a matrix into a file
    Return 1 if the file could not be opened, and 0 if the file was writen correctly
*/
int writeMatrix(char * filename, const matrix_t * matrix)
{
    int i, j;
    FILE* file_ptr = fopen(filename, "w");
    // Validate that the file could be opened
    if (file_ptr == NULL)
    {
        printf("ERROR: Unable to open file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    // Write the header with the size of the matrix
    fprintf(file_ptr, "%d %d\n", matrix->rows, matrix->cols);

    for (i = 0; i < matrix->rows; i++)
    {
        for (j = 0; j < matrix->cols; j++)
        {
            fprintf(file_ptr, "%.2f ", matrix->data[i][j]);
        }
        fprintf(file_ptr, "\n");
    }

    fclose(file_ptr);

    return 0;
}
