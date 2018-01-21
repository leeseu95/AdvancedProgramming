//Seung Hoon Lee Kim
//A01021720
//Assignment 2 - Numerical Base Conversion

#ifndef NUMERICAL_H
#define NUMERICAL_H

#include <stdio.h>
#include <string.h>

typedef struct {
    int base;
    union {
        int decimal;
        char resString[50];
    } u;
} numerical;

int changeVal(char str);

void convertChar(char *str, int base, numerical * num);

char changeChar(int num);

void reverseString(char *str);

void convertNum(char res[], int base, int input, numerical * num);

#endif