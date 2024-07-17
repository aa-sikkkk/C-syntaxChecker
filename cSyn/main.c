// Author: Aas1kkk
// Date: 2024-07-13
// Description: A a tool designed to analyze and validate the syntax of C and C++ codebases. It ensures code quality by detecting common syntax errors and providing detailed reports.
// File version: 1.1
// Last Update: 2024-07-17
// License: GNU License
// Recent changes: Added support for C++ specific constructs and improved the output format.

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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    char *input_filename = argv[1];
    FILE *input_file;
    FILE *output_file;
    FileLine lines[100];
    char buffer[1024];
    int total_lines = 0, line_length, comment_position;
    int is_cpp = 0;

    // Determine file type based on extension
    if (strstr(input_filename, ".cpp") != NULL) {
        is_cpp = 1;
    } else if (strstr(input_filename, ".c") == NULL) {
        printf("Error: Unsupported file extension. Please use .c or .cpp files.\n");
        return 1;
    }

    input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error: Could not open input file.\n");
        return 1;
    }

    output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        printf("Error: Could not open output file.\n");
        fclose(input_file);
        return 1;
    }

    // Read lines from the input file
    while (fgets(buffer, sizeof(buffer), input_file) != NULL) {
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
    int function_count = 0, prototype_count = 0;
    for (int i = 0; i < total_lines; i++) {
        if (strchr(lines[i].line_text, '(') && strchr(lines[i].line_text, ')')) {
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

// Function to check the usage of specific keywords
void check_keyword_usage(FileLine lines[], int total_lines, FILE *output_file, int is_cpp) {
    const char *keywords_c[] = {"printf", "scanf", "fgets", "fputs"};
    const char *keywords_cpp[] = {"cout", "cin", "std::"};
    int keyword_count_c = sizeof(keywords_c) / sizeof(keywords_c[0]);
    int keyword_count_cpp = sizeof(keywords_cpp) / sizeof(keywords_cpp[0]);

    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < keyword_count_c; j++) {
            if (strstr(lines[i].line_text, keywords_c[j])) {
                fprintf(output_file, "Line %d: Found keyword usage '%s'\n", lines[i].line_number, keywords_c[j]);
            }
        }
        if (is_cpp) {
            for (int j = 0; j < keyword_count_cpp; j++) {
                if (strstr(lines[i].line_text, keywords_cpp[j])) {
                    fprintf(output_file, "Line %d: Found keyword usage '%s'\n", lines[i].line_number, keywords_cpp[j]);
                }
            }
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

// Function to check for print and scan functions
void check_print_scan_functions(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (is_print_function(lines[i].line_text, lines[i].line_length)) {
            fprintf(output_file, "Line %d: Found print function usage\n", lines[i].line_number);
        }
        if (is_scan_function(lines[i].line_text, lines[i].line_length)) {
            fprintf(output_file, "Line %d: Found scan function usage\n", lines[i].line_number);
        }
    }
}

// Function to check if a line contains a print function
int is_print_function(char line[], int line_length) {
    const char *print_functions[] = {"printf", "puts", "putchar", "cout"};
    int function_count = sizeof(print_functions) / sizeof(print_functions[0]);

    for (int i = 0; i < function_count; i++) {
        if (strstr(line, print_functions[i])) {
            return 1;
        }
    }
    return 0;
}

// Function to check if a line contains a scan function
int is_scan_function(char line[], int line_length) {
    const char *scan_functions[] = {"scanf", "gets", "getchar", "cin"};
    int function_count = sizeof(scan_functions) / sizeof(scan_functions[0]);

    for (int i = 0; i < function_count; i++) {
        if (strstr(line, scan_functions[i])) {
            return 1;
        }
    }
    return 0;
}

// Function to count variables in the code
void count_variables(FileLine lines[], int total_lines, FILE *output_file) {
    int variable_count = 0;
    for (int i = 0; i < total_lines; i++) {
        if (is_valid_variable_declaration(lines[i].line_text)) {
            variable_count++;
        }
    }
    fprintf(output_file, "Number of variables: %d\n", variable_count);
}

// Function to check for file operations
void check_file_operations(FileLine lines[], int total_lines, FILE *output_file) {
    const char *file_functions[] = {"fopen", "fclose", "fread", "fwrite", "fprintf", "fscanf"};
    int function_count = sizeof(file_functions) / sizeof(file_functions[0]);

    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < function_count; j++) {
            if (strstr(lines[i].line_text, file_functions[j])) {
                fprintf(output_file, "Line %d: Found file operation '%s'\n", lines[i].line_number, file_functions[j]);
            }
        }
    }
}

// Function to trim leading and trailing whitespace from a string
void trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char) *str)) str++;

    // All spaces?
    if (*str == 0) return;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char) *end)) end--;

    // Write new null terminator
    *(end + 1) = 0;
}

int is_comment_line(const char *line) {
    return strstr(line, "//") == line || strstr(line, "/*") == line;
}

int is_multiline_comment_start(const char *line) {
    return strstr(line, "/*") != NULL;
}

int is_multiline_comment_end(const char *line) {
    return strstr(line, "*/") != NULL;
}

// Function to check for semicolons at the end of lines
void check_semicolons(FileLine lines[], int total_lines, FILE *output_file) {
    int in_multiline_comment = 0;

    for (int i = 0; i < total_lines; i++) {
        trim_whitespace(lines[i].line_text);

        if (strlen(lines[i].line_text) == 0) {
            continue; // Skip empty lines
        }

        if (is_comment_line(lines[i].line_text)) {
            continue; // Skip single-line comments
        }

        if (is_multiline_comment_start(lines[i].line_text)) {
            in_multiline_comment = 1; // Entering a multiline comment
        }

        if (in_multiline_comment) {
            if (is_multiline_comment_end(lines[i].line_text)) {
                in_multiline_comment = 0; // Exiting a multiline comment
            }
            continue; // Skip the content of multiline comments
        }

        int len = strlen(lines[i].line_text);
        if (lines[i].line_text[len - 1] != ';' &&
            strstr(lines[i].line_text, "if") == NULL &&
            strstr(lines[i].line_text, "for") == NULL &&
            strstr(lines[i].line_text, "while") == NULL &&
            strstr(lines[i].line_text, "switch") == NULL &&
            strstr(lines[i].line_text, "else") == NULL &&
            strstr(lines[i].line_text, "{") == NULL &&
            strstr(lines[i].line_text, "}") == NULL &&
            lines[i].line_text[len - 1] != '}') {
            fprintf(output_file, "Line %d: Missing semicolon at the end\n", lines[i].line_number);
        }
    }
}

// Function to check for C++ specific constructs
void check_cpp_specific_constructs(FileLine lines[], int total_lines, FILE *output_file) {
    check_class_usage(lines, total_lines, output_file);
    check_templates(lines, total_lines, output_file);
}

// Function to check for class usage
void check_class_usage(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (strstr(lines[i].line_text, "class")) {
            fprintf(output_file, "Line %d: Found class declaration\n", lines[i].line_number);
        }
    }
}

// Function to check for templates
void check_templates(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (strstr(lines[i].line_text, "template") && strstr(lines[i].line_text, "<")) {
            fprintf(output_file, "Line %d: Found template declaration\n", lines[i].line_number);
        }
    }
}

// Function to check if a line contains a for loop
int is_for_loop(char *line, int length) {
    return strstr(line, "for") != NULL;
}

// Function to check if a line contains a while loop
int is_while_loop(char *line, int length) {
    return strstr(line, "while") != NULL;
}

// Function to check if a line contains valid function syntax
int is_valid_function_syntax(char *line) {
    return strchr(line, '(') != NULL && strchr(line, ')') != NULL;
}

// Function to check if a line contains a valid variable declaration
int is_valid_variable_declaration(char *line) {
    const char *types[] = {"int", "float", "char", "double", "short", "long"};
    int type_count = sizeof(types) / sizeof(types[0]);

    for (int i = 0; i < type_count; i++) {
        if (strstr(line, types[i])) {
            return 1;
        }
    }
    return 0;
}
