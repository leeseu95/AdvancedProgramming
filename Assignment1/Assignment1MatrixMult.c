//Seung Hoon Lee Kim
//A01021720
//Assignment 1 - Matrix Multiplication

#include <stdio.h>

void matrixMult(int rowA, int colA, int rowB, int colB, float mat1[rowA][colA], float mat2[rowB][colB], float matRes[rowA][colB]);

int main() {
    int rowA, colA, rowB, colB; //Initialize variables that we can use later for rows,cols of matrix
    FILE *matAFile; //These are the variables for the files to open (both matrixes A and B)
    FILE *matBFile;

    //The next two lines are the ones where the txt name will have to be changed
    matAFile = fopen("Matrix_B.txt", "r"); //We utilize fopen (r stands for reading the file)
    matBFile = fopen("Matrix_A.txt", "r"); //Same for matrixB

    //Verify that the files were opened correctly 
    if(matAFile && matBFile) {
        //We open the files
        fscanf(matAFile, "%d", &rowA); //We store the first two numbers in the text file as rowA and colA
        fscanf(matAFile, "%d", &colA);

        //Initialize some other variables
        float temp = 0; //Initialize a temp variable which we'll be using later in our cycle
        float matA[rowA][colA];

        for (int i = 0; i < rowA; i++) {
            for (int j = 0; j < colA; j++) {
                fscanf(matAFile, "%f", &temp); //We store the number in a temp so we can use it later and put it into our matrix
                matA[i][j] = temp;
                // printf("%.2f  ", matA[i][j]); //Print to debug our matrix to see if it's correct
            }
            printf("\n"); //Print a line for each row done
        }
        fclose(matAFile); // WE close the file at the end of its usage

        //We use the same process for our matrix B
        fscanf(matBFile, "%d", &rowB); //We store the first two numbers in the text file as rowA and colA
        fscanf(matBFile, "%d", &colB);
        float matB[rowB][colB];
        
        for (int i = 0; i < rowB; i++) {
            for (int j = 0; j < colB; j++) {
                fscanf(matBFile, "%f", &temp); //We store the number in a temp so we can use it later and put it into our matrix
                matB[i][j] = temp;
                // printf("%.2f  ", matB[i][j]); //Print to debug our matrix to see if it's correct
            }
            printf("\n"); //Print a line for each row done
        }
        fclose(matBFile); // WE close the file at the end of its usage

        float matRes[rowA][colB]; //Create the matrix in which we'll be putting our answer
        matrixMult(rowA, colA, rowB, colB, matA, matB, matRes); //Call our matrix multiplier function
    } else {
        printf("Either of the files were not available to be opened. Please check that the file names are correct.\n"); //In case the files were not able to open
    }
}

void matrixMult(int rowA, int colA, int rowB, int colB, float mat1[rowA][colA], float mat2[rowB][colB], float matRes[rowA][colB]){
    float temp = 0; //We create a temp to put in our numbers into our resulting matrix
    
    // printf("%d", colA); //Debugging
    // printf("%d", rowB);
    if(colA == rowB) { //Validate that both of our matrixes are multipliable
        for (int i = 0; i < rowA; i++) { //Start the first cycle based on the rows of A
            for (int j = 0; j < colB; j++) { //Start second cycle
                for (int k = 0; k < colB; k++) { //Start third cycle
                    temp += mat1[i][k] * mat2[k][j]; //Set our temp according to how the matrixes are multiplied
                }
                matRes[i][j] = temp; //Put in our matrix in
                temp = 0; //Reinitialize our temp to 0
            }
        }
        for (int i = 0; i < rowA; i++) {
            for (int j = 0; j < colB; j++) {
                printf("%.2f  ", matRes[i][j]); //Print to debug our matrix to see if it's correct
            }
            printf("\n"); //Print a line for each row done
        }
    } else {
        printf("Your matrixes cannot be multiplied to to their sizes, please check the sizes again.\n"); // In case the matrixes arent able to be multiplied
    }
}