#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Structure to store each line of the file along with its line number and length
typedef struct {
    int line_number;
    int line_length;
    char line_text[100];
} FileLine;

// Function declarations
void print_lines(FileLine lines[], int total_lines, FILE *output_file);
int find_comment_position(char line[], int line_length);
void check_brackets(FileLine lines[], int total_lines, FILE *output_file);
void check_keywords(FileLine lines[], int total_lines, FILE *output_file);
void count_functions_and_prototypes(FileLine lines[], int total_lines, FILE *output_file);
void check_keyword_usage(FileLine lines[], int total_lines, FILE *output_file);
void check_builtin_functions(FileLine lines[], int total_lines, FILE *output_file);
void check_print_scan_functions(FileLine lines[], int total_lines, FILE *output_file);
int is_print_function(char line[], int line_length);
int is_scan_function(char line[], int line_length);
void count_variables(FileLine lines[], int total_lines, FILE *output_file);
int is_gets_function(char line[], int line_length);
int is_puts_function(char line[], int line_length);
int is_fprintf_function(char line[], int line_length);
int is_fscanf_function(char line[], int line_length);
void check_file_operations(FileLine lines[], int total_lines, FILE *output_file);
int is_for_loop(char line[], int line_length);
int is_while_loop(char line[], int line_length);
int is_valid_function_syntax(char *line);
int is_valid_variable_declaration(char *line);

int main() {
    FILE *input_file;
    FILE *output_file;
    FileLine lines[100];
    char buffer[100];
    int total_lines, i = 0, line_length, comment_position;

    input_file = fopen("input.txt", "r");
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
    while (fgets(buffer, 100, input_file) != NULL) {
        line_length = strlen(buffer); // Get the length of the line
        comment_position = find_comment_position(buffer, line_length); // Find position of comment if exists

        // If the line is not empty and does not contain a comment
        if (buffer[0] != '\n' && comment_position == -1) {
            lines[i].line_number = i + 1;
            lines[i].line_length = line_length;
            strcpy(lines[i].line_text, buffer);
            i++;
        } else if (buffer[0] != '\n' && comment_position != -1) {
            lines[i].line_number = i + 1;
            for (int j = 0; j < comment_position; j++) {
                lines[i].line_text[j] = buffer[j];
            }
            lines[i].line_text[comment_position] = '\0';
            lines[i].line_length = comment_position;
            i++;
        }
    }
    total_lines = i;

    fclose(input_file);

    // Perform various checks and write results to the output file
    print_lines(lines, total_lines, output_file);
    check_brackets(lines, total_lines, output_file);
    check_keywords(lines, total_lines, output_file);
    count_functions_and_prototypes(lines, total_lines, output_file);
    check_keyword_usage(lines, total_lines, output_file);
    check_builtin_functions(lines, total_lines, output_file);
    check_print_scan_functions(lines, total_lines, output_file);
    count_variables(lines, total_lines, output_file);
    check_file_operations(lines, total_lines, output_file);

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
void check_keywords(FileLine lines[], int total_lines, FILE *output_file) {
    const char *keywords[] = {"int", "float", "if", "else", "while", "for", "return"};
    int keyword_count = sizeof(keywords) / sizeof(keywords[0]);
    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < keyword_count; j++) {
            if (strstr(lines[i].line_text, keywords[j])) {
                fprintf(output_file, "Line %d: Found keyword '%s'\n", lines[i].line_number, keywords[j]);
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
void check_keyword_usage(FileLine lines[], int total_lines, FILE *output_file) {
    const char *keywords[] = {"printf", "scanf", "fgets", "fputs"};
    int keyword_count = sizeof(keywords) / sizeof(keywords[0]);
    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < keyword_count; j++) {
            if (strstr(lines[i].line_text, keywords[j])) {
                fprintf(output_file, "Line %d: Found keyword usage '%s'\n", lines[i].line_number, keywords[j]);
            }
        }
    }
}

// Function to check for usage of built-in functions
void check_builtin_functions(FileLine lines[], int total_lines, FILE *output_file) {
    const char *builtin_functions[] = {"malloc", "free", "exit", "strcpy", "strlen"};
    int builtin_count = sizeof(builtin_functions) / sizeof(builtin_functions[0]);
    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < builtin_count; j++) {
            if (strstr(lines[i].line_text, builtin_functions[j])) {
                fprintf(output_file, "Line %d: Found built-in function usage '%s'\n", lines[i].line_number, builtin_functions[j]);
            }
        }
    }
}

// Function to check for print and scan functions
void check_print_scan_functions(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (is_print_function(lines[i].line_text, lines[i].line_length)) {
            fprintf(output_file, "Line %d: Found print function.\n", lines[i].line_number);
        }
        if (is_scan_function(lines[i].line_text, lines[i].line_length)) {
            fprintf(output_file, "Line %d: Found scan function.\n", lines[i].line_number);
        }
    }
}

// Function to check if a line is a print function
int is_print_function(char line[], int line_length) {
    return strstr(line, "printf") != NULL || strstr(line, "fprintf") != NULL || strstr(line, "puts") != NULL;
}

// Function to check if a line is a scan function
int is_scan_function(char line[], int line_length) {
    return strstr(line, "scanf") != NULL || strstr(line, "fscanf") != NULL || strstr(line, "gets") != NULL;
}

// Function to count variable declarations
void count_variables(FileLine lines[], int total_lines, FILE *output_file) {
    int variable_count = 0;
    for (int i = 0; i < total_lines; i++) {
        if (is_valid_variable_declaration(lines[i].line_text)) {
            variable_count++;
        }
    }
    fprintf(output_file, "Number of variables: %d\n", variable_count);
}

// Function to check if a line contains the gets function
int is_gets_function(char line[], int line_length) {
    return strstr(line, "gets") != NULL;
}

// Function to check if a line contains the puts function
int is_puts_function(char line[], int line_length) {
    return strstr(line, "puts") != NULL;
}

// Function to check if a line contains the fprintf function
int is_fprintf_function(char line[], int line_length) {
    return strstr(line, "fprintf") != NULL;
}

// Function to check if a line contains the fscanf function
int is_fscanf_function(char line[], int line_length) {
    return strstr(line, "fscanf") != NULL;
}

// Function to check for file operations
void check_file_operations(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (is_fprintf_function(lines[i].line_text, lines[i].line_length)) {
            fprintf(output_file, "Line %d: Found fprintf function.\n", lines[i].line_number);
        }
        if (is_fscanf_function(lines[i].line_text, lines[i].line_length)) {
            fprintf(output_file, "Line %d: Found fscanf function.\n", lines[i].line_number);
        }
    }
}

// Function to check if a line contains a for loop
int is_for_loop(char line[], int line_length) {
    return strstr(line, "for") != NULL;
}

// Function to check if a line contains a while loop
int is_while_loop(char line[], int line_length) {
    return strstr(line, "while") != NULL;
}

// Function to check for valid function syntax
int is_valid_function_syntax(char *line) {
    return strchr(line, '(') != NULL && strchr(line, ')') != NULL && strchr(line, '{') != NULL;
}

// Function to check for valid variable declaration syntax
int is_valid_variable_declaration(char *line) {
    const char *types[] = {"int", "float", "char", "double", "long", "short"};
    int type_count = sizeof(types) / sizeof(types[0]);
    for (int i = 0; i < type_count; i++) {
        if (strstr(line, types[i]) != NULL) {
            return 1;
        }
    }
    return 0;
}
