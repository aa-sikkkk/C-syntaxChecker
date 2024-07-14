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
int is_for_loop(char line[], int line_length);
int is_while_loop(char line[], int line_length);
int is_valid_function_syntax(char *line);
int is_valid_variable_declaration(char *line);
void check_semicolons(FileLine lines[], int total_lines, FILE *output_file);
void check_cpp_specific_constructs(FileLine lines[], int total_lines, FILE *output_file);
void check_class_usage(FileLine lines[], int total_lines, FILE *output_file);
void check_templates(FileLine lines[], int total_lines, FILE *output_file);
int is_for_loop(char *line, int length);
int is_while_loop(char *line, int length);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    char *input_filename = argv[1];
    FILE *input_file;
    FILE *output_file;
    FileLine lines[100];
    char buffer[100];
    int total_lines, i = 0, line_length, comment_position;
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

// Function to check the usage of built-in functions
void check_builtin_functions(FileLine lines[], int total_lines, FILE *output_file, int is_cpp) {
    const char *functions_c[] = {"malloc", "free", "strcpy", "strlen", "strcmp"};
    const char *functions_cpp[] = {"std::string", "std::vector", "std::map", "std::set"};
    int function_count_c = sizeof(functions_c) / sizeof(functions_c[0]);
    int function_count_cpp = sizeof(functions_cpp) / sizeof(functions_cpp[0]);

    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < function_count_c; j++) {
            if (strstr(lines[i].line_text, functions_c[j])) {
                fprintf(output_file, "Line %d: Found built-in function '%s'\n", lines[i].line_number, functions_c[j]);
            }
        }
        if (is_cpp) {
            for (int j = 0; j < function_count_cpp; j++) {
                if (strstr(lines[i].line_text, functions_cpp[j])) {
                    fprintf(output_file, "Line %d: Found built-in function '%s'\n", lines[i].line_number, functions_cpp[j]);
                }
            }
        }
    }
}

// Function to check the usage of printf and scanf functions
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

// Function to count variables
void count_variables(FileLine lines[], int total_lines, FILE *output_file) {
    int variable_count = 0;
    for (int i = 0; i < total_lines; i++) {
        if (is_valid_variable_declaration(lines[i].line_text)) {
            variable_count++;
        }
    }
    fprintf(output_file, "Number of variables: %d\n", variable_count);
}

// Function to check the usage of file operations
void check_file_operations(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (strstr(lines[i].line_text, "fopen") || strstr(lines[i].line_text, "fclose")) {
            fprintf(output_file, "Line %d: Found file operation function.\n", lines[i].line_number);
        }
    }
}

int is_for_loop(char *line, int length) {
    return strstr(line, "for") != NULL && strchr(line, '(') != NULL && strchr(line, ')') != NULL;
}

int is_while_loop(char *line, int length) {
    return strstr(line, "while") != NULL && strchr(line, '(') != NULL && strchr(line, ')') != NULL;
}

// Function to check if a line contains a valid function syntax
int is_valid_function_syntax(char *line) {
    if (strchr(line, '(') && strchr(line, ')')) {
        return 1;
    }
    return 0;
}

// Function to check if a line contains a valid variable declaration
int is_valid_variable_declaration(char *line) {
    const char *types[] = {"int", "float", "char", "double", "long", "short", "unsigned", "signed"};
    int type_count = sizeof(types) / sizeof(types[0]);

    for (int i = 0; i < type_count; i++) {
        if (strstr(line, types[i])) {
            if (strchr(line, ';')) {
                return 1;
            }
        }
    }
    return 0;
}

// Function to check for missing semicolons
void check_semicolons(FileLine lines[], int total_lines, FILE *output_file) {
    int in_multiline_comment = 0;

    for (int i = 0; i < total_lines; i++) {
        char *line = lines[i].line_text;
        int line_length = strlen(line);

        // Skip empty lines and header files
        if (line_length == 0 || strstr(line, "#include")) {
            continue;
        }

        // Handle single-line comments
        char *single_comment = strstr(line, "//");
        if (single_comment) {
            *single_comment = '\0';
            line_length = single_comment - line;
        }

        // Handle multi-line comments
        char *block_comment_start = strstr(line, "/*");
        if (block_comment_start) {
            in_multiline_comment = 1;
            *block_comment_start = '\0';
            line_length = block_comment_start - line;
        }

        // Check for end of multi-line comments
        if (in_multiline_comment) {
            if (strstr(line, "*/")) {
                in_multiline_comment = 0;
                char *block_comment_end = strstr(line, "*/");
                line_length = block_comment_end - line + 2;
            } else {
                continue; // Skip this line
            }
        }

        // Trim whitespace
        while (line_length > 0 && isspace(line[line_length - 1])) {
            line[line_length - 1] = '\0';
            line_length--;
        }

        // Check for missing semicolon
        if (line_length > 0) {
            char last_char = line[line_length - 1];

            // Ignore lines that end with ;, {, or }
            if (last_char != ';' && last_char != '{' && last_char != '}') {
                // Check if it is a line that requires a semicolon
                if (!is_for_loop(line, line_length) && !is_while_loop(line, line_length) &&
                    !strstr(line, "if") && !strstr(line, "return") &&
                    !strstr(line, "switch") && !strstr(line, "case")) {
                    fprintf(output_file, "Line %d: Missing semicolon.\n", lines[i].line_number);
                }
            }
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
            fprintf(output_file, "Line %d: Found class definition.\n", lines[i].line_number);
        }
    }
}

// Function to check for template usage in C++
void check_templates(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (strstr(lines[i].line_text, "template")) {
            fprintf(output_file, "Line %d: Found template definition.\n", lines[i].line_number);
        }
    }
}
