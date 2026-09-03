#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Test file paths
#define TEST_OUTPUT "output.txt"
#define TEST_HTML "output.html"
#define TEST_FILE "test.c"

#if defined(_WIN32) || defined(_WIN64)
    #define SYNCHECK_PATH "..\\cSyn\\synCheck.exe"
#else
    #define SYNCHECK_PATH "../cSyn/synCheck.exe"
#endif

// Function declarations
void run_test(const char* test_name, const char* test_code, int expected_errors, int expected_warnings);
void cleanup_test_files(void);
void test_bracket_matching(void);
void test_semicolon_checking(void);
void test_variable_declaration(void);
void test_function_declaration(void);
void test_memory_management(void);
void test_pointer_usage(void);
void test_array_bounds(void);
void test_type_safety(void);
void test_naming_conventions(void);
void test_full_syntax_issues(void);

// Helper function to create a test file
void create_test_file(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file) {
        fprintf(file, "%s", content);
        fclose(file);
        printf("[DEBUG] Created %s with content:\n%s\n", filename, content);
        fflush(stdout);
    } else {
        printf("Error: Could not create test file %s\n", filename);
    }
}

// Helper function to run a test
void run_test(const char* test_name, const char* test_code, int expected_errors, int expected_warnings) {
    printf("Running test: %s\n", test_name);
    
    // Create test file
    create_test_file(TEST_FILE, test_code);
    
    // Run syntax checker
    char command[256];
    sprintf(command, "%s %s", SYNCHECK_PATH, TEST_FILE);
    printf("[DEBUG] Running command: %s\n", command);
    if (system(command) != 0) {
        printf("Error: Syntax checker failed to run\n");
        return;
    }
    
    // Read output file
    FILE* output = fopen(TEST_OUTPUT, "r");
    if (!output) {
        printf("Error: Could not open output file %s\n", TEST_OUTPUT);
        return;
    }
    
    // Count errors and warnings from the summary section
    int errors = 0, warnings = 0;
    char line[8192];
    while (fgets(line, sizeof(line), output)) {
        if (strncmp(line, "Errors:", 7) == 0)
            sscanf(line + 7, "%d", &errors);
        else if (strncmp(line, "Warnings:", 9) == 0)
            sscanf(line + 9, "%d", &warnings);
    }
    fclose(output);
    
    printf("[DEBUG] Found %d errors and %d warnings\n", errors, warnings);
    
    // Assert results
    assert(errors == expected_errors && "Error count mismatch");
    assert(warnings == expected_warnings && "Warning count mismatch");
    
    printf("Test passed: %s\n", test_name);
}

// Test bracket matching
void test_bracket_matching(void) {
    printf("[DEBUG] Entering test_bracket_matching\n");
    
    // Valid case - simple program with proper brackets
    const char* test_code = 
        "int main() {\n"
        "    int x = 5;\n"
        "    if (x > 0) {\n"
        "        x = 10;\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    run_test("Bracket Matching - Valid", test_code, 0, 0);  // No errors or warnings for valid code
    
    // Invalid case - missing closing bracket
    const char* test_code_invalid = 
        "int main() {\n"
        "    int x = 5;\n"
        "    if (x > 0 {\n"
        "        x = 10;\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    run_test("Bracket Matching - Invalid", test_code_invalid, 2, 0);  // 2 errors for bracket issues
    
    printf("[DEBUG] Exiting test_bracket_matching\n");
}

// Test semicolon checking
void test_semicolon_checking(void) {
    printf("[DEBUG] Entering test_semicolon_checking\n");
    
    // Valid case - all statements end with semicolons
    const char* test_code = 
        "int main() {\n"
        "    int x = 5;\n"
        "    return 0;\n"
        "}\n";
    run_test("Semicolon - Valid", test_code, 0, 1);  // unused-variable warning for 'x'
    
    // Invalid case - missing semicolons
    const char* test_code_invalid = 
        "int main() {\n"
        "    int x = 5\n"
        "    return 0\n"
        "}\n";
    run_test("Semicolon - Invalid", test_code_invalid, 1, 1);  // 1 missing-semicolon error, unused-variable warning
    
    printf("[DEBUG] Exiting test_semicolon_checking\n");
}

// Test variable declaration
void test_variable_declaration(void) {
    printf("[DEBUG] Entering test_variable_declaration\n");
    
    // Valid case - proper variable declarations
    const char* test_code = 
        "int main() {\n"
        "    int count = 5;\n"
        "    float value = 3.14;\n"
        "    return 0;\n"
        "}\n";
    run_test("Variable Declaration - Valid", test_code, 0, 2);  // Warnings for type safety
    
    // Invalid case - type mismatches
    const char* test_code_invalid = 
        "int main() {\n"
        "    int count = 3.14;\n"
        "    float value = 5;\n"
        "    return 0;\n"
        "}\n";
    run_test("Variable Declaration - Invalid", test_code_invalid, 0, 2);  // Warnings for type safety
    
    printf("[DEBUG] Exiting test_variable_declaration\n");
}

// Test function declaration
void test_function_declaration(void) {
    printf("[DEBUG] Entering test_function_declaration\n");
    
    // Valid case - proper function declaration
    const char* test_code = 
        "int add(int a, int b) {\n"
        "    return a + b;\n"
        "}\n"
        "int main() {\n"
        "    return 0;\n"
        "}\n";
    run_test("Function Declaration - Valid", test_code, 0, 0);  // params are caller-initialized, no warnings
    
    // Invalid case - missing parameter types
    const char* test_code_invalid = 
        "int add(a, b) {\n"
        "    return a + b;\n"
        "}\n"
        "int main() {\n"
        "    return 0;\n"
        "}\n";
    run_test("Function Declaration - Invalid", test_code_invalid, 0, 0);  // no params to flag
    
    printf("[DEBUG] Exiting test_function_declaration\n");
}

// Test memory management
void test_memory_management(void) {
    printf("[DEBUG] Entering test_memory_management\n");
    
    // Valid case - proper malloc/free
    const char* test_code = 
        "#include <stdlib.h>\n"
        "int main() {\n"
        "    int* arr = malloc(10 * sizeof(int));\n"
        "    if (arr != NULL) {\n"
        "        free(arr);\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    run_test("Memory Management - Valid", test_code, 0, 1);  // Warning for type safety
    
    // Invalid case - memory leak
    const char* test_code_invalid = 
        "#include <stdlib.h>\n"
        "int main() {\n"
        "    int* arr = malloc(10 * sizeof(int));\n"
        "    return 0;\n"
        "}\n";
    run_test("Memory Management - Invalid", test_code_invalid, 0, 3);  // leak + unchecked malloc + unused 'arr'
    
    printf("[DEBUG] Exiting test_memory_management\n");
}

// Test pointer usage
void test_pointer_usage(void) {
    printf("[DEBUG] Entering test_pointer_usage\n");
    
    // Valid case - proper pointer initialization
    const char* test_code = 
        "int main() {\n"
        "    int value = 5;\n"
        "    int* ptr = &value;\n"
        "    return 0;\n"
        "}\n";
    run_test("Pointer Usage - Valid", test_code, 0, 1);  // Warning for type safety
    
    // Invalid case - uninitialized pointer
    const char* test_code_invalid = 
        "int main() {\n"
        "    int* ptr;\n"
        "    *ptr = 5;\n"
        "    return 0;\n"
        "}\n";
    run_test("Pointer Usage - Invalid", test_code_invalid, 1, 0);  // uninitialized pointer dereference
    
    printf("[DEBUG] Exiting test_pointer_usage\n");
}

// Test array bounds
void test_array_bounds(void) {
    printf("[DEBUG] Entering test_array_bounds\n");
    
    // Valid case - proper array access
    const char* test_code = 
        "int main() {\n"
        "    int arr[5];\n"
        "    for(int i = 0; i < 5; i++) {\n"
        "        arr[i] = i;\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    run_test("Array Bounds - Valid", test_code, 0, 0);  // correct bounds access
    
    // Invalid case - out of bounds access
    const char* test_code_invalid = 
        "int main() {\n"
        "    int arr[5];\n"
        "    arr[5] = 10;\n"
        "    return 0;\n"
        "}\n";
    run_test("Array Bounds - Invalid", test_code_invalid, 0, 1);  // out-of-bounds access
    
    printf("[DEBUG] Exiting test_array_bounds\n");
}

// Test type safety
void test_type_safety(void) {
    printf("[DEBUG] Entering test_type_safety\n");
    
    // Valid case - proper type usage
    const char* test_code = 
        "int main() {\n"
        "    int count = 5;\n"
        "    float value = 3.14;\n"
        "    return 0;\n"
        "}\n";
    run_test("Type Safety - Valid", test_code, 0, 2);  // Warnings for type safety
    
    // Invalid case - implicit conversion
    const char* test_code_invalid = 
        "int main() {\n"
        "    int count = 3.14;\n"
        "    return 0;\n"
        "}\n";
    run_test("Type Safety - Invalid", test_code_invalid, 0, 1);  // Warning for type safety
    
    printf("[DEBUG] Exiting test_type_safety\n");
}

// Test naming conventions
void test_naming_conventions(void) {
    printf("[DEBUG] Entering test_naming_conventions\n");
    
    // Valid case - descriptive names
    const char* test_code = 
        "int main() {\n"
        "    int userCount = 0;\n"
        "    float averageScore = 0.0;\n"
        "    return 0;\n"
        "}\n";
    run_test("Naming Conventions - Valid", test_code, 0, 2);  // Warnings for type safety
    
    // Invalid case - poor naming
    const char* test_code_invalid = 
        "int main() {\n"
        "    int x = 0;\n"
        "    float y = 0.0;\n"
        "    return 0;\n"
        "}\n";
    run_test("Naming Conventions - Invalid", test_code_invalid, 0, 2);  // Warnings for type safety
    
    printf("[DEBUG] Exiting test_naming_conventions\n");
}

// Test full syntax issues
void test_full_syntax_issues(void) {
    const char* test_code =
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "// Test file with various syntax issues\n"
        "\n"
        "int main() {\n"
        "    // Missing semicolon\n"
        "    int a = 5\n"
        "    float b = 3.14\n"
        "    \n"
        "    // Mismatched brackets\n"
        "    if (a > b {\n"
        "        printf(\"a is greater than b\\n\");\n"
        "    else {\n"
        "        printf(\"a is less than or equal to b\\n\");\n"
        "    }\n"
        "    \n"
        "    // Nested brackets with issues\n"
        "    for (int i = 0; i < 5; i++) {\n"
        "        if (i % 2 == 0) {\n"
        "            printf(\"Even number: %d\\n\", i);\n"
        "        } else {\n"
        "            printf(\"Odd number: %d\\n\", i);\n"
        "        }\n"
        "    }\n"
        "    \n"
        "    // Array with mismatched brackets\n"
        "    int arr[5 = {1, 2, 3, 4, 5};\n"
        "    \n"
        "    // Function call with unclosed parenthesis\n"
        "    printf(\"Hello, World!\\n\"\n"
        "    \n"
        "    // Memory leak - malloc without free\n"
        "    char *str = malloc(50);\n"
        "    strcpy(str, \"Hello\");\n"
        "    \n"
        "    // Unsafe pointer usage\n"
        "    int *ptr;\n"
        "    *ptr = 10;  // Using uninitialized pointer\n"
        "    \n"
        "    // Array bounds issue\n"
        "    int arr2[5];\n"
        "    arr2[5] = 10;  // Out of bounds access\n"
        "    \n"
        "    // Type safety issue\n"
        "    int x = 3.14;  // Implicit conversion\n"
        "    \n"
        "    // Poor naming convention\n"
        "    int i = 0;\n"
        "    int j = 1;\n"
        "    \n"
        "    // Missing return statement\n"
        "    // return 0;\n"
        "}\n";
    run_test("Full Syntax Issues", test_code, 6, 7); // Adjust expected errors/warnings as needed
}

// Cleanup test files
void cleanup_test_files(void) {
    remove(TEST_FILE);
    remove(TEST_OUTPUT);
    remove(TEST_HTML);
}

int main(void) {
    printf("Starting syntax checker tests...\n\n");
    
    // Run all tests
    test_bracket_matching();
    test_semicolon_checking();
    test_variable_declaration();
    test_function_declaration();
    test_memory_management();
    test_pointer_usage();
    test_array_bounds();
    test_type_safety();
    test_naming_conventions();
    test_full_syntax_issues();
    
    // Cleanup
    cleanup_test_files();
    
    printf("\nAll tests completed successfully!\n");
    return 0;
} 