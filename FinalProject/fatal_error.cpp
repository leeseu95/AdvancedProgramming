/*
Final Project
Seung Hoon Lee - A01021720
Samuel Torrero - A01361063
*/

#include "fatal_error.h"

void fatalError(const char * message)
{
    perror(message);
    exit(EXIT_FAILURE);
}
