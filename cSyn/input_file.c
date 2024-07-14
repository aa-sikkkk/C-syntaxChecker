#include <stdio.h>
#include <stdlib.h>

int main() {
    int a = 5;
    int b = 10;
    printf("This is a test program.\n")
    if (a > b {
        printf("a is greater than b\n");
    else {
        printf("a is less than or equal to b\n");
    }
    floot c = 3.14;
    prntf("Value of c: %f\n", c);

    for int i = 0; i < 5; i++) {
        printf("i = %d\n", i);
    }

    // Missing function prototype
    int result = multiply(a, b);
    printf("Result: %d\n", result);

    // Incorrect function definition
    int multiply(int x, int y) {
        return x * y;
    }

    // Incorrect array declaration
    int arr[5 = {1, 2, 3, 4, 5};

    // Incorrect pointer usage
    int *p = &a;
    printf("Value pointed by p: %d\n", *p);
    p + 1; // This does nothing useful

    // Memory allocation without proper cast
    char *str = malloc(50);
    strcpy(str, "Hello, World!");

    // Incorrect condition in while loop
    while (a < 20);
        printf("a is less than 20\n");

    // Unused variables
    int x, y;

    // Return statement outside function
    return 0;

int multiply(int x, int y) {
    return x * y;
}

// Missing main function closing bracket