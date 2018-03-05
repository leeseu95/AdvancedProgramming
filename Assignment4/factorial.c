//Seung Hoon Lee Kim
//A01021720
//Assignemnt 4 - Factorial Forks

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int factorial(int num);

int main(int argc, char **argv) {
    if (argc == 2){//If there's a a parameter passed on to the call function we use that
        int num = atoi(argv[1]); //Change the parameter we passed from char to int
        printf("Factorial of %d: %d\n", num, factorial(num)); //Print our factorial number

    } else {
        printf("The parameters are incorrect\nCorrect format: ./factorial num\n"); //In case the arguments are wrong, print the correct way
    }
}

int factorial(int num) {
    int res = 1; //Our result will start at 1

    if(num == 0) { //If the number is 0, we return a 1
        return 1;
    } else if (num < 0) { //If the number is a negative number, we exit the program
        printf("Your number is a negative number, exiting program\n");
        return 0;
    }
    for(int i = 1; i <= num; i++) { //For loop starting from 1 until the number
        res = res * i;
    }

    return res; //Return our result
}