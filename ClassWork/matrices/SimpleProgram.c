#include <stdio.h>

//Constant Value
#define PI 3.1413
//Preprocessor Macro
#define square(x) ((x)*(x))

//Another way to write the macro as inline function
// inline float square(float num)
// {
//     return num * num;
// }

int main() {
    printf("Hello there\n");
    printf("This is PI: %f\n", PI);
    printf("Square of PI: %f\n", square(PI));
    return 0;
}