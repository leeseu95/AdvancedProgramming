//Seung Hoon Lee Kim
//A01021720
//Assignment 2 - Numerical Base Conversion

#include "numerical.c" //Include our class
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if(argc < 6) { //getOpt, if we have less than 6 arguments
        printf("Please use the correct way to run the program ./Executable -i <InputBase> -o <OutputBase> <Numbers>\n");
        exit(1);
    }

    numerical num;

    int c; //getopt Manual
    int iflag = 0; //Flags for getopt
    int oflag = 0;
    int wantedBase = 0; //Wanted base

    while((c = getopt(argc, argv, "i:o:")) != -1) {
        switch (c) {
            case 'i' :
                if (iflag) {
                    printf("Please use the correct way to run the program ./Executable -i <InputBase> -o <OutputBase> <Numbers>\n");
                    iflag ++;
                    exit(1);
                }
                else
                    num.base = atof(optarg);
            case 'o' :
                if (iflag) {
                    printf("Please use the correct way to run the program ./Executable -i <InputBase> -o <OutputBase> <Numbers>\n");
                    oflag ++;
                    exit(1);
                }
                else
                    wantedBase = atof(optarg);
                break;
            default : 
                printf("Please use the correct way to run the program ./Executable -i <InputBase> -o <OutputBase> <Numbers>\n");
                exit(1);
        }
    }

    // for (int index = optind; index < argc ; index++) {
    //     printf("This is what's inside the argv %s\n", argv[index]);
    // } //Debugging purposes

    if(num.base == 10) {
        int temp;
        for (int index = optind; index < argc; index++){ //Iterate through what we havei nside the arguments   
            temp = atof(argv[index]); //We use our temp to not overwrite what's in num union's data
            printf("Converting %d in base %d to base %d is", temp, num.base, wantedBase);
            convertNum(num.u.resString, wantedBase, temp, &num);    //Convert our number to string
            printf(" %s\n", num.u.resString);
        }
    }
    else if(num.base != 10 && wantedBase == 10) {
        for (int index = optind; index < argc; index++) {
            strcpy(num.u.resString, argv[index]);
            printf("Converting %s in base %d to decimal is", num.u.resString, num.base); //Convert whats in our string to a decimal
            convertChar(num.u.resString, num.base, & num); //and print it later
            printf(" %d\n", num.u.decimal);
        }
    }
    else {
        char temp[10]; //We use a temp to not overwrite whats in the union
        for (int index = optind; index < argc; index++) {
            strcpy(num.u.resString, argv[index]); 
            strcpy(temp, num.u.resString);
            convertChar(num.u.resString, num.base, &num); // First convert to base x to base 10
            printf("Converting %s in base ", temp);
            printf("%d to base %d is", num.base, wantedBase);
            convertNum(temp, wantedBase, num.u.decimal, &num);  //Convert from base 10 to base y
            printf(" %s\n", num.u.resString);
        }
    }

    return 0;
}