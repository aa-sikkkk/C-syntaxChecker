// Author: Aas1kkk
// Date: 2024-07-13
// Description: A tool designed to analyze and validate the syntax of C and C++ codebases. It ensures code quality by detecting common syntax errors and providing detailed reports.
// File version: 1.2
// Last Update: 2024-07-17
// License: GNU License
// Recent changes: Added support for multiple file inputs, C++ specific constructs, improved output format, and added cyclomatic complexity calculation.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Structure to store each line of the file along with its line number and length
typedef struct {
    int line_number;
    int line_length;
    char line_text[1024];
} FileLine;

// Function declarations
void print_lines(FileLine lines[], int total_lines, FILE *output_file);
int find_comment_position(char line[], int line_length);
void check_brackets(FileLine lines[], int total_lines, FILE *output_file);
void check_keywords(FileLine lines[], int total_lines, FILE *output_file, int is_cpp);
void count_functions_and_prototypes(FileLine lines[], int total_lines, FILE *output_file);
void check_keyword_usage(FileLine lines[], int total_lines, FILE *output_file, int is_cpp);
void check_builtin_functions(FileLine lines[], int total_lines, FILE *output_file, int is_cpp);
void check_print_scan_functions(FileLine lines[], int total_lines, FILE *output_file);
int is_print_function(char line[], int line_length);
int is_scan_function(char line[], int line_length);
void count_variables(FileLine lines[], int total_lines, FILE *output_file);
void check_file_operations(FileLine lines[], int total_lines, FILE *output_file);
int is_for_loop(char *line, int length);
int is_while_loop(char *line, int length);
int is_valid_function_syntax(char *line);
int is_valid_variable_declaration(char *line);
void check_semicolons(FileLine lines[], int total_lines, FILE *output_file);
void check_cpp_specific_constructs(FileLine lines[], int total_lines, FILE *output_file);
void check_class_usage(FileLine lines[], int total_lines, FILE *output_file);
void check_templates(FileLine lines[], int total_lines, FILE *output_file);
void analyze_file(const char *input_filename, FILE *output_file);
int calculate_cyclomatic_complexity(FileLine lines[], int total_lines);

// Function to process a single file
// Function to process a single file
void analyze_file(const char *input_filename, FILE *output_file) {
    FILE *input_file;
    FileLine *lines = NULL;
    char buffer[1024];
    int total_lines = 0, line_length, comment_position;
    int is_cpp = 0;
    int capacity = 100;  // Initial capacity

    lines = (FileLine *)malloc(capacity * sizeof(FileLine));
    if (lines == NULL) {
        fprintf(output_file, "Error: Memory allocation failed.\n");
        return;
    }

    // Determine file type based on extension
    if (strstr(input_filename, ".cpp") != NULL) {
        is_cpp = 1;
    } else if (strstr(input_filename, ".c") == NULL) {
        fprintf(output_file, "Error: Unsupported file extension for file %s. Please use .c or .cpp files.\n", input_filename);
        free(lines);
        return;
    }

    input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        fprintf(output_file, "Error: Could not open input file %s.\n", input_filename);
        free(lines);
        return;
    }

    // Read lines from the input file
    while (fgets(buffer, sizeof(buffer), input_file) != NULL) {
        if (total_lines >= capacity) {
            capacity *= 2;
            lines = (FileLine *)realloc(lines, capacity * sizeof(FileLine));
            if (lines == NULL) {
                fprintf(output_file, "Error: Memory reallocation failed.\n");
                fclose(input_file);
                return;
            }
        }

        line_length = strlen(buffer); // Get the length of the line
        comment_position = find_comment_position(buffer, line_length); // Find position of comment if exists

        // Process the line based on the presence of comments
        if (buffer[0] != '\n' && comment_position == -1) {
            lines[total_lines].line_number = total_lines + 1;
            lines[total_lines].line_length = line_length;
            strcpy(lines[total_lines].line_text, buffer);
            total_lines++;
        } else if (buffer[0] != '\n' && comment_position != -1) {
            lines[total_lines].line_number = total_lines + 1;
            strncpy(lines[total_lines].line_text, buffer, comment_position);
            lines[total_lines].line_text[comment_position] = '\0';
            lines[total_lines].line_length = comment_position;
            total_lines++;
        }
    }

    fclose(input_file);

    // Perform various checks and write results to the output file
    fprintf(output_file, "Analysis for file: %s\n", input_filename);
    print_lines(lines, total_lines, output_file);
    check_brackets(lines, total_lines, output_file);
    check_keywords(lines, total_lines, output_file, is_cpp);
    count_functions_and_prototypes(lines, total_lines, output_file);
    check_keyword_usage(lines, total_lines, output_file, is_cpp);
    check_builtin_functions(lines, total_lines, output_file, is_cpp);
    check_print_scan_functions(lines, total_lines, output_file);
    count_variables(lines, total_lines, output_file);
    check_file_operations(lines, total_lines, output_file);
    check_semicolons(lines, total_lines, output_file);

    if (is_cpp) {
        check_cpp_specific_constructs(lines, total_lines, output_file);
    }

    int cyclomatic_complexity = calculate_cyclomatic_complexity(lines, total_lines);
    fprintf(output_file, "Cyclomatic Complexity: %d\n", cyclomatic_complexity);

    fprintf(output_file, "\n");

    // Free allocated memory
    free(lines);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <source_file1> <source_file2> ... <source_fileN>\n", argv[0]);
        return 1;
    }

    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        printf("Error: Could not open output file.\n");
        return 1;
    }

    // Process each file passed as an argument
    for (int i = 1; i < argc; i++) {
        analyze_file(argv[i], output_file);
    }

    fclose(output_file);

    return 0;
}

// Function to print the lines
void print_lines(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        fprintf(output_file, "Line %d: %s", lines[i].line_number, lines[i].line_text);
    }
}

// Function to find the position of a comment in a line
int find_comment_position(char line[], int line_length) {
    for (int i = 0; i < line_length - 1; i++) {
        if (line[i] == '/' && line[i + 1] == '/') {
            return i;
        }
    }
    return -1;
}

// Function to check for matching brackets
void check_brackets(FileLine lines[], int total_lines, FILE *output_file) {
    int open_brackets = 0, close_brackets = 0;
    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < lines[i].line_length; j++) {
            if (lines[i].line_text[j] == '{') open_brackets++;
            if (lines[i].line_text[j] == '}') close_brackets++;
        }
    }
    if (open_brackets != close_brackets) {
        fprintf(output_file, "Error: Mismatched brackets detected.\n");
    } else {
        fprintf(output_file, "Brackets are balanced.\n");
    }
}

// Function to check for specific keywords in the code
void check_keywords(FileLine lines[], int total_lines, FILE *output_file, int is_cpp) {
    const char *keywords_c[] = {"int", "float", "if", "else", "while", "for", "return"};
    const char *keywords_cpp[] = {"class", "public", "private", "protected", "new", "delete", "namespace", "template"};
    int keyword_count_c = sizeof(keywords_c) / sizeof(keywords_c[0]);
    int keyword_count_cpp = sizeof(keywords_cpp) / sizeof(keywords_cpp[0]);

    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < keyword_count_c; j++) {
            if (strstr(lines[i].line_text, keywords_c[j])) {
                fprintf(output_file, "Line %d: Found keyword '%s'\n", lines[i].line_number, keywords_c[j]);
            }
        }
        if (is_cpp) {
            for (int j = 0; j < keyword_count_cpp; j++) {
                if (strstr(lines[i].line_text, keywords_cpp[j])) {
                    fprintf(output_file, "Line %d: Found keyword '%s'\n", lines[i].line_number, keywords_cpp[j]);
                }
            }
        }
    }
}

// Function to count functions and prototypes
void count_functions_and_prototypes(FileLine lines[], int total_lines, FILE *output_file) {
    int function_count = 0;
    int prototype_count = 0;

    for (int i = 0; i < total_lines; i++) {
        if (is_valid_function_syntax(lines[i].line_text)) {
            if (strchr(lines[i].line_text, ';')) {
                prototype_count++;
            } else {
                function_count++;
            }
        }
    }
    fprintf(output_file, "Number of functions: %d\n", function_count);
    fprintf(output_file, "Number of function prototypes: %d\n", prototype_count);
}

// Function to check keyword usage
void check_keyword_usage(FileLine lines[], int total_lines, FILE *output_file, int is_cpp) {
    for (int i = 0; i < total_lines; i++) {
        if (is_for_loop(lines[i].line_text, lines[i].line_length)) {
            fprintf(output_file, "Line %d: Contains a for loop\n", lines[i].line_number);
        }
        if (is_while_loop(lines[i].line_text, lines[i].line_length)) {
            fprintf(output_file, "Line %d: Contains a while loop\n", lines[i].line_number);
        }
    }
}

// Function to check for the usage of built-in functions
void check_builtin_functions(FileLine lines[], int total_lines, FILE *output_file, int is_cpp) {
    const char *builtin_functions_c[] = {"malloc", "calloc", "free", "exit", "qsort", "bsearch"};
    const char *builtin_functions_cpp[] = {"new", "delete"};
    int function_count_c = sizeof(builtin_functions_c) / sizeof(builtin_functions_c[0]);
    int function_count_cpp = sizeof(builtin_functions_cpp) / sizeof(builtin_functions_cpp[0]);

    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < function_count_c; j++) {
            if (strstr(lines[i].line_text, builtin_functions_c[j])) {
                fprintf(output_file, "Line %d: Found built-in function usage '%s'\n", lines[i].line_number, builtin_functions_c[j]);
            }
        }
        if (is_cpp) {
            for (int j = 0; j < function_count_cpp; j++) {
                if (strstr(lines[i].line_text, builtin_functions_cpp[j])) {
                    fprintf(output_file, "Line %d: Found built-in function usage '%s'\n", lines[i].line_number, builtin_functions_cpp[j]);
                }
            }
        }
    }
}


// Function to check the usage of print and scan functions
void check_print_scan_functions(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (is_print_function(lines[i].line_text, lines[i].line_length)) {
            fprintf(output_file, "Line %d: Contains a print function\n", lines[i].line_number);
        }
        if (is_scan_function(lines[i].line_text, lines[i].line_length)) {
            fprintf(output_file, "Line %d: Contains a scan function\n", lines[i].line_number);
        }
    }
}

// Function to check if a line contains a print function
int is_print_function(char line[], int line_length) {
    if (strstr(line, "printf") || strstr(line, "cout")) {
        return 1;
    }
    return 0;
}

// Function to check if a line contains a scan function
int is_scan_function(char line[], int line_length) {
    if (strstr(line, "scanf") || strstr(line, "cin")) {
        return 1;
    }
    return 0;
}

// Function to count the number of variables
void count_variables(FileLine lines[], int total_lines, FILE *output_file) {
    const char *data_types[] = {"int", "float", "double", "char"};
    int data_type_count = sizeof(data_types) / sizeof(data_types[0]);
    int variable_count = 0;

    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < data_type_count; j++) {
            if (strstr(lines[i].line_text, data_types[j])) {
                variable_count++;
                break;
            }
        }
    }
    fprintf(output_file, "Number of variables: %d\n", variable_count);
}

// Function to check file operations
void check_file_operations(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (strstr(lines[i].line_text, "fopen")) {
            fprintf(output_file, "Line %d: Contains a file open operation\n", lines[i].line_number);
        }
        if (strstr(lines[i].line_text, "fclose")) {
            fprintf(output_file, "Line %d: Contains a file close operation\n", lines[i].line_number);
        }
    }
}

// Function to check if a line contains a for loop
int is_for_loop(char *line, int length) {
    if (strstr(line, "for")) {
        return 1;
    }
    return 0;
}

// Function to check if a line contains a while loop
int is_while_loop(char *line, int length) {
    if (strstr(line, "while")) {
        return 1;
    }
    return 0;
}

// Function to check if a line has valid function syntax
int is_valid_function_syntax(char *line) {
    if (strchr(line, '(') && strchr(line, ')') && strchr(line, '{')) {
        return 1;
    }
    return 0;
}

// Function to check if a line has valid variable declaration
int is_valid_variable_declaration(char *line) {
    const char *data_types[] = {"int", "float", "double", "char"};
    int data_type_count = sizeof(data_types) / sizeof(data_types[0]);

    for (int i = 0; i < data_type_count; i++) {
        if (strstr(line, data_types[i])) {
            return 1;
        }
    }
    return 0;
}

// Function to check for missing semicolons
void check_semicolons(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (!strchr(lines[i].line_text, ';') && !strstr(lines[i].line_text, "for") && !strstr(lines[i].line_text, "while") && !strstr(lines[i].line_text, "{") && !strstr(lines[i].line_text, "}")) {
            fprintf(output_file, "Line %d: Missing semicolon\n", lines[i].line_number);
        }
    }
}

// Function to check for C++ specific constructs
void check_cpp_specific_constructs(FileLine lines[], int total_lines, FILE *output_file) {
    check_class_usage(lines, total_lines, output_file);
    check_templates(lines, total_lines, output_file);
}

// Function to check for class usage in C++
void check_class_usage(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (strstr(lines[i].line_text, "class")) {
            fprintf(output_file, "Line %d: Contains class declaration\n", lines[i].line_number);
        }
    }
}

// Function to check for template usage in C++
void check_templates(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (strstr(lines[i].line_text, "template")) {
            fprintf(output_file, "Line %d: Contains template usage\n", lines[i].line_number);
        }
    }
}

// Function to calculate cyclomatic complexity
int calculate_cyclomatic_complexity(FileLine lines[], int total_lines) {
    int complexity = 1; // Cyclomatic complexity starts at 1
    for (int i = 0; i < total_lines; i++) {
        if (strstr(lines[i].line_text, "if") || strstr(lines[i].line_text, "else if") ||
            strstr(lines[i].line_text, "for") || strstr(lines[i].line_text, "while") ||
            strstr(lines[i].line_text, "case") || strstr(lines[i].line_text, "default") ||
            strstr(lines[i].line_text, "&&") || strstr(lines[i].line_text, "||")) {
            complexity++;
        }
    }
    return complexity;
}