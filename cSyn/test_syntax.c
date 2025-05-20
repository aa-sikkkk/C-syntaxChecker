#include <stdio.h>
#include <stdlib.h>

// Test file with various syntax issues

int main() {
    // Missing semicolon
    int a = 5
    float b = 3.14
    
    // Mismatched brackets
    if (a > b {
        printf("a is greater than b\n");
    else {
        printf("a is less than or equal to b\n");
    }
    
    // Nested brackets with issues
    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0) {
            printf("Even number: %d\n", i);
        } else {
            printf("Odd number: %d\n", i);
        }
    }
    
    // Array with mismatched brackets
    int arr[5 = {1, 2, 3, 4, 5};
    
    // Function call with unclosed parenthesis
    printf("Hello, World!\n"
    
    // Memory leak - malloc without free
    char *str = malloc(50);
    strcpy(str, "Hello");
    
    // Unsafe pointer usage
    int *ptr;
    *ptr = 10;  // Using uninitialized pointer
    
    // Array bounds issue
    int arr2[5];
    arr2[5] = 10;  // Out of bounds access
    
    // Type safety issue
    int x = 3.14;  // Implicit conversion
    
    // Poor naming convention
    int i = 0;
    int j = 1;
    
    // Missing return statement
    // return 0;
} 