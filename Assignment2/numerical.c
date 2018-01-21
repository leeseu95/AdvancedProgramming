//Seung Hoon Lee Kim
//A01021720
//Assignment 2 - Numerical Base Conversion

#define _CRT_SECURE_NO_DEPRECATE

#include "numerical.h"

//Functions to change from any base to base 10
int changeVal(char str) {
    if (str >= '0' && str <= '9') //If the string that we're passing has a value between 0 and 9 we return the value itself as int
        return (int)str - '0';
    else //Else we return the Value of the letter (i.e. B would be 11) add 1 after every letter for Abc, etc.
        return (int)str - 'A' + 10;
}

void convertChar(char *str, int base, numerical * num) {
    int length = strlen(str); //WE declare the length of the string that we just passed
    int pow; //Initialize the base power and our ersult we'll be storing it on

    pow = 1; //Start the base power with 1  
    int res = 0;
    for (int i = length - 1; i >= 0; i--) { //WE initialize a cycle starting from the length -1 till it reaches 0
        if (changeVal(str[i]) >= base) { //Validate that the number is bigger than our base
            printf("Invalid Number, please enter a number that's bigger than the base\n");
            break;
        }

        res += changeVal(str[i]) * pow;
        pow = pow * base; //We change our power every cycle by multiplying it by current pow by base
    }
    num->u.decimal = res;
}

//Functions to change from base 10 to any other base
char changeChar(int num) {
    if (num >= 0 && num <= 9) //For any of the numbers between 0 and 9 we return the num + a '0'
        return (char)(num + '0'); 
    else //If that's not the case a letter
        return (char)(num - 10 + 'A');
}

void reverseString(char *str) {
    int length = strlen(str); //We declare the length of the string that we passed
    for (int i = 0; i < length/2; i++) {
        char temp = str[i]; //We create a temporal to swap around in our characters to reverse
        str[i] = str[length-i-1];
        str[length-i-1] = temp;
    }
}

void convertNum(char res[], int base, int input, numerical * num) {
    int index = 0; //We declare the index that we'll be using
    while (input > 0) {
        res[index++] = changeChar(input % base); //We change thep osition in index++ to decimal to char with our function
        input /= base; //Divide the input by the base
    }
    res[index] = '\0';
    reverseString(res); //Reverse the string that we created
    strcpy(num->u.resString, res);
}