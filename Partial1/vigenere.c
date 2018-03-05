//Seung Hoon Lee Kim
//A01021720
//Partial 1
//Vigenere cipher/decipher code

//Libraries to include
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//Declaration of our functions
void vigenereCipher(char * filename, char* keyText); 
void vigenereDecipher(char * filename, char* keyText);


int main(int argc, char **argv) { //Our main which will be recieving the parameters necessary

    char * filePlain = "aladdin.txt"; //In case we only get 1 extra parameter, we use aladdin.txt
    char * fileEncoded = "encoded.txt";

    char * keyText = argv[3]; //We use a string to pass the parameter 

    if (argc == 4) { //If we do get 3 extra parameters (option, key, file name)
        int opt = atoi(argv[1]);
        filePlain = argv[2]; //We name our filetext to read, whatever the third parameter is
        
        if (opt == 1 ) { //If our option is ciphering then
            vigenereCipher(filePlain, keyText);
        } else if (opt == 2) {
            vigenereDecipher(fileEncoded, keyText);
        } else {
            printf("Incorrect option input");
        }

    } else if (argc == 3) { //If only 2 extra parameter (option, key)
        int opt = atoi(argv[1]);
        if (opt == 1) {
            vigenereCipher(filePlain, keyText);
        } else if (opt == 2) {
            vigenereDecipher(fileEncoded, keyText);
        } else {
            printf("Incorrect option input");
        }
    } else { //If no parameter = incorrect syntax
        printf("Parameters are incorrect, please do it this way: ./Executable keyword file.txt");
    }

    return 0;
}

void vigenereCipher(char * filename, char* keyText) {
    FILE * file = fopen(filename, "rb");
    FILE * fileWrite = fopen("encoded.txt", "w");
    
    long fsize;
    char * buffer = 0; //String array into which we'll be saving the whole text
    
    if (file) { //If our file exists
        fseek(file, 0 , SEEK_END);
        fsize = ftell(file);
        fseek(file, 0, SEEK_SET);
        buffer = malloc(fsize);
        if (buffer) { //If our string buffer exists
            fread(buffer, 1, fsize, file);
        }
        fclose(file);
    }

    // printf("\n%s", keyText); //Debugging purposes
    // printf("\n%s", buffer);

    int keySize = strlen(keyText); //Varaible that contains the length of our keytext used in our for
    
    for (int i = 0; i < keySize; i++) { //Validate that our key string is composed of alphabetical letters
        if((keyText[i] >= '0') && (keyText[i] <= '9')){
            printf("Teacher, you didn't ask for this, this is cheating Mr. Echeverria\n"); //Message if that happens
            exit(0);
        }
    }

    int indexKey = 0; //Variable index to use in our for loop for our key array
    int textSize = strlen(buffer);

    for (int i = 0; i < textSize; i++) {
        // if (indexKey >= strlen(keyText)) //If our index key is bigger than our length of the key we reinitiate it at 0
        //     indexKey = 0;

        if (isalpha(buffer[i])) { //If our character in the buffer is alphabetical
            //Variable to calcualte ASCII Code later
            char referenceValue = 'a';

            //Use the mathematical code of using formula ci = (mi + kj) mod 26 (taken from wikipedia)
            char cipheredChar = ((buffer[i] - referenceValue + (keyText[(indexKey % keySize)] - referenceValue)) % 26) + referenceValue;
            fprintf(fileWrite, "%c", cipheredChar);

            //Increment our key index position
            indexKey ++;

        } else { //If it's not, we just print the character that is in that place
            fprintf(fileWrite, "%c", buffer[i]);
        }
    }

    printf("\nEncrpyted text written to file 'encoded.txt'\n\n");
    fclose(fileWrite);
    free(buffer);
}   

//We apply the same logic of ciphering in this one
void vigenereDecipher(char * filename, char* keyText) {
    FILE * file = fopen(filename, "rb");
    FILE * fileWrite = fopen("decoded.txt", "w");

    long fsize;
    char * buffer = 0; //String array into which we'll be saving the whole text
    
    if (file) { //If our file exists
        fseek(file, 0 , SEEK_END);
        fsize = ftell(file);
        fseek(file, 0, SEEK_SET);
        buffer = malloc(fsize);
        if (buffer) { //If our string buffer exists
            fread(buffer, 1, fsize, file);
        }
        fclose(file);
    }

    // printf("\n%s", keyText); //Debugging purposes
    // printf("\n%s", buffer);

    int keySize = strlen(keyText); //Varaible that contains the length of our keytext used in our for
    
    for (int i = 0; i < keySize; i++) { //Validate that our key string is composed of alphabetical letters
        if((keyText[i] >= '0') && (keyText[i] <= '9')){
            printf("Teacher, you didn't ask for this, this is cheating Mr. Echeverria\n"); //Message if that happens
            exit(0);
        }
    }

    int indexKey = 0; //Variable index to use in our for loop for our key array
    int textSize = strlen(buffer);

    for (int i = 0; i < textSize; i++) {
        // if (indexKey >= strlen(keyText)) //If our index key is bigger than our length of the key we reinitiate it at 0
        //     indexKey = 0;

        if (isalpha(buffer[i])) { //If our character in the buffer is alphabetical
            //Variable to calcualte ASCII Code later
            char referenceValue = 'a';

            //We use the
            char cipheredChar = (referenceValue + (((buffer[i] - keyText[indexKey % keySize]) + 26) % 26));
            fprintf(fileWrite, "%c", cipheredChar);

            //Increment our key index position
            indexKey ++;

        } else { //If it's not, we just print the character that is in that place
            fprintf(fileWrite, "%c", buffer[i]);
        }
    }
    printf("\nDeciphered text written to file 'decoded.txt'\n\n");
    fclose(fileWrite);
    free(buffer);
}   