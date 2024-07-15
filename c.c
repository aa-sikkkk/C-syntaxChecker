#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"

// Structure to store each line of the file along with its line number and length
typedef struct {
    int line_number;
    int line_length;
    char line_text[1024];
} FileLine;

// Function declarations
void print_lines(FileLine lines[], int total_lines, FILE *output_file, int is_cpp);
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
void apply_syntax_highlighting(char *line, char *highlighted_line, int is_cpp);
void detect_unused_variables(FileLine lines[], int total_lines, FILE *output_file);
int calculate_cyclomatic_complexity(FileLine lines[], int total_lines);
void analyze_function_complexity(FileLine lines[], int total_lines, FILE *output_file);
void check_memory_management_issues(FileLine lines[], int total_lines, FILE *output_file);
int find_block_comment_end(char line[], int line_length);
void handle_block_comments(FileLine lines[], int total_lines, FILE *output_file);

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
        }
    }

    total_lines = i; // Number of valid lines

    // Perform syntax analysis
    print_lines(lines, total_lines, output_file, is_cpp);
    check_brackets(lines, total_lines, output_file);
    check_keywords(lines, total_lines, output_file, is_cpp);
    count_functions_and_prototypes(lines, total_lines, output_file);
    check_keyword_usage(lines, total_lines, output_file, is_cpp);
    check_builtin_functions(lines, total_lines, output_file, is_cpp);
    check_print_scan_functions(lines, total_lines, output_file);
    count_variables(lines, total_lines, output_file);
    check_file_operations(lines, total_lines, output_file);
    check_cpp_specific_constructs(lines, total_lines, output_file);
    check_semicolons(lines, total_lines, output_file);
    detect_unused_variables(lines, total_lines, output_file);
    analyze_function_complexity(lines, total_lines, output_file);
    check_memory_management_issues(lines, total_lines, output_file);
    handle_block_comments(lines, total_lines, output_file);

    fclose(input_file);
    fclose(output_file);

    return 0;
}

void apply_syntax_highlighting(char *line, char *highlighted_line, int is_cpp) {
    char *keywords_c[] = {"int", "float", "if", "else", "while", "for", "return", "void", "char", "double", "long", "short", "unsigned", "signed"};
    char *keywords_cpp[] = {"class", "public", "private", "protected", "new", "delete", "namespace", "template"};
    int keyword_count_c = sizeof(keywords_c) / sizeof(keywords_c[0]);
    int keyword_count_cpp = sizeof(keywords_cpp) / sizeof(keywords_cpp[0]);

    int i = 0, j = 0, k;
    while (line[i] != '\0') {
        // Check for comments
        if (line[i] == '/' && line[i + 1] == '/') {
            strcat(highlighted_line, GREEN);
            strcat(highlighted_line, &line[i]);
            strcat(highlighted_line, RESET);
            break;
        }

        // Check for string literals
        if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i];
            strcat(highlighted_line, YELLOW);
            highlighted_line[j++] = line[i++];
            while (line[i] != '\0' && line[i] != quote) {
                highlighted_line[j++] = line[i++];
            }
            if (line[i] == quote) {
                highlighted_line[j++] = line[i++];
            }
            strcat(highlighted_line, RESET);
            continue;
        }

        // Check for keywords
        int is_keyword = 0;
        for (k = 0; k < keyword_count_c; k++) {
            int len = strlen(keywords_c[k]);
            if (strncmp(&line[i], keywords_c[k], len) == 0 && !isalnum(line[i + len]) && line[i + len] != '_') {
                strcat(highlighted_line, CYAN);
                strncat(highlighted_line, &line[i], len);
                strcat(highlighted_line, RESET);
                i += len;
                j += len;
                is_keyword = 1;
                break;
            }
        }
        if (is_cpp && !is_keyword) {
            for (k = 0; k < keyword_count_cpp; k++) {
                int len = strlen(keywords_cpp[k]);
                if (strncmp(&line[i], keywords_cpp[k], len) == 0 && !isalnum(line[i + len]) && line[i + len] != '_') {
                    strcat(highlighted_line, MAGENTA);
                    strncat(highlighted_line, &line[i], len);
                    strcat(highlighted_line, RESET);
                    i += len;
                    j += len;
                    is_keyword = 1;
                    break;
                }
            }
        }
        if (is_keyword) continue;

        // Copy other characters
        highlighted_line[j++] = line[i++];
    }
    highlighted_line[j] = '\0';
}

void print_lines(FileLine lines[], int total_lines, FILE *output_file, int is_cpp) {
    char highlighted_line[2048];
    for (int i = 0; i < total_lines; i++) {
        highlighted_line[0] = '\0';
        apply_syntax_highlighting(lines[i].line_text, highlighted_line, is_cpp);
        fprintf(output_file, "Line %d: %s\n", lines[i].line_number, highlighted_line);
    }
}

int find_comment_position(char line[], int line_length) {
    // Check for single line comment
    for (int i = 0; i < line_length - 1; i++) {
        if (line[i] == '/' && line[i + 1] == '/') {
            return i;
        }
    }
    return -1;
}

void check_brackets(FileLine lines[], int total_lines, FILE *output_file) {
    int open_brackets = 0, close_brackets = 0;
    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < lines[i].line_length; j++) {
            if (lines[i].line_text[j] == '{') {
                open_brackets++;
            } else if (lines[i].line_text[j] == '}') {
                close_brackets++;
            }
        }
    }
    fprintf(output_file, "Brackets: Open - %d, Close - %d\n", open_brackets, close_brackets);
}

void check_keywords(FileLine lines[], int total_lines, FILE *output_file, int is_cpp) {
    char *keywords[] = {"int", "float", "if", "else", "while", "for", "return", "void", "char", "double", "long", "short", "unsigned", "signed"};
    int keyword_counts[13] = {0};
    char *cpp_keywords[] = {"class", "public", "private", "protected", "new", "delete", "namespace", "template"};
    int cpp_keyword_counts[8] = {0};

    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < 13; j++) {
            if (strstr(lines[i].line_text, keywords[j])) {
                keyword_counts[j]++;
            }
        }
        if (is_cpp) {
            for (int j = 0; j < 8; j++) {
                if (strstr(lines[i].line_text, cpp_keywords[j])) {
                    cpp_keyword_counts[j]++;
                }
            }
        }
    }

    fprintf(output_file, "Keyword Usage:\n");
    for (int i = 0; i < 13; i++) {
        fprintf(output_file, "%s: %d\n", keywords[i], keyword_counts[i]);
    }
    if (is_cpp) {
        for (int i = 0; i < 8; i++) {
            fprintf(output_file, "%s: %d\n", cpp_keywords[i], cpp_keyword_counts[i]);
        }
    }
}

void count_functions_and_prototypes(FileLine lines[], int total_lines, FILE *output_file) {
    int function_count = 0, prototype_count = 0;
    for (int i = 0; i < total_lines; i++) {
        if (is_valid_function_syntax(lines[i].line_text)) {
            function_count++;
        }
        if (strstr(lines[i].line_text, ";") && !strstr(lines[i].line_text, "printf") && !strstr(lines[i].line_text, "scanf")) {
            prototype_count++;
        }
    }
    fprintf(output_file, "Function Count: %d\n", function_count);
    fprintf(output_file, "Prototype Count: %d\n", prototype_count);
}

void check_keyword_usage(FileLine lines[], int total_lines, FILE *output_file, int is_cpp) {
    // Same as check_keywords function, can be merged with it
}

void check_builtin_functions(FileLine lines[], int total_lines, FILE *output_file, int is_cpp) {
    char *functions[] = {"printf", "scanf", "malloc", "free", "strcpy", "strlen", "strcmp"};
    int function_counts[7] = {0};
    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < 7; j++) {
            if (strstr(lines[i].line_text, functions[j])) {
                function_counts[j]++;
            }
        }
    }
    fprintf(output_file, "Builtin Function Usage:\n");
    for (int i = 0; i < 7; i++) {
        fprintf(output_file, "%s: %d\n", functions[i], function_counts[i]);
    }
}

void check_print_scan_functions(FileLine lines[], int total_lines, FILE *output_file) {
    int print_count = 0, scan_count = 0;
    for (int i = 0; i < total_lines; i++) {
        if (is_print_function(lines[i].line_text, lines[i].line_length)) {
            print_count++;
        }
        if (is_scan_function(lines[i].line_text, lines[i].line_length)) {
            scan_count++;
        }
    }
    fprintf(output_file, "Printf Count: %d\n", print_count);
    fprintf(output_file, "Scanf Count: %d\n", scan_count);
}

int is_print_function(char line[], int line_length) {
    return strstr(line, "printf") != NULL;
}

int is_scan_function(char line[], int line_length) {
    return strstr(line, "scanf") != NULL;
}

void count_variables(FileLine lines[], int total_lines, FILE *output_file) {
    int variable_count = 0;
    for (int i = 0; i < total_lines; i++) {
        if (is_valid_variable_declaration(lines[i].line_text)) {
            variable_count++;
        }
    }
    fprintf(output_file, "Variable Count: %d\n", variable_count);
}

void check_file_operations(FileLine lines[], int total_lines, FILE *output_file) {
    int fopen_count = 0, fclose_count = 0;
    for (int i = 0; i < total_lines; i++) {
        if (strstr(lines[i].line_text, "fopen")) {
            fopen_count++;
        }
        if (strstr(lines[i].line_text, "fclose")) {
            fclose_count++;
        }
    }
    fprintf(output_file, "File Operations:\n");
    fprintf(output_file, "fopen: %d\n", fopen_count);
    fprintf(output_file, "fclose: %d\n", fclose_count);
}

void check_semicolons(FileLine lines[], int total_lines, FILE *output_file) {
    int missing_semicolon_count = 0;
    for (int i = 0; i < total_lines; i++) {
        if (lines[i].line_text[lines[i].line_length - 2] != ';' && lines[i].line_text[lines[i].line_length - 2] != '{' && lines[i].line_text[lines[i].line_length - 2] != '}') {
            missing_semicolon_count++;
        }
    }
    fprintf(output_file, "Missing Semicolons: %d\n", missing_semicolon_count);
}

void check_cpp_specific_constructs(FileLine lines[], int total_lines, FILE *output_file) {
    check_class_usage(lines, total_lines, output_file);
    check_templates(lines, total_lines, output_file);
}

void check_class_usage(FileLine lines[], int total_lines, FILE *output_file) {
    int class_count = 0;
    for (int i = 0; i < total_lines; i++) {
        if (strstr(lines[i].line_text, "class")) {
            class_count++;
        }
    }
    fprintf(output_file, "Class Usage: %d\n", class_count);
}

void check_templates(FileLine lines[], int total_lines, FILE *output_file) {
    int template_count = 0;
    for (int i = 0; i < total_lines; i++) {
        if (strstr(lines[i].line_text, "template")) {
            template_count++;
        }
    }
    fprintf(output_file, "Template Usage: %d\n", template_count);
}

void detect_unused_variables(FileLine lines[], int total_lines, FILE *output_file) {
    // Implementation can be complex, typically involves symbol table construction and usage analysis
    // For simplicity, we assume we have a function that does this
}

void analyze_function_complexity(FileLine lines[], int total_lines, FILE *output_file) {
    int complexity = calculate_cyclomatic_complexity(lines, total_lines);
    fprintf(output_file, "Cyclomatic Complexity: %d\n", complexity);
}

void check_memory_management_issues(FileLine lines[], int total_lines, FILE *output_file) {
    // Implementation involves scanning for malloc/free and ensuring proper pairing and error checking
}

void handle_block_comments(FileLine lines[], int total_lines, FILE *output_file) {
    int in_block_comment = 0;
    for (int i = 0; i < total_lines; i++) {
        if (in_block_comment) {
            int end_pos = find_block_comment_end(lines[i].line_text, lines[i].line_length);
            if (end_pos != -1) {
                in_block_comment = 0;
            }
            continue;
        }
        if (strstr(lines[i].line_text, "/*")) {
            in_block_comment = 1;
            int end_pos = find_block_comment_end(lines[i].line_text, lines[i].line_length);
            if (end_pos != -1) {
                in_block_comment = 0;
            }
        }
    }
    fprintf(output_file, "Handled block comments.\n");
}

int find_block_comment_end(char line[], int line_length) {
    for (int i = 0; i < line_length - 1; i++) {
        if (line[i] == '*' && line[i + 1] == '/') {
            return i;
        }
    }
    return -1;
}

int calculate_cyclomatic_complexity(FileLine lines[], int total_lines) {
    int complexity = 1; // Initial complexity is 1
    for (int i = 0; i < total_lines; i++) {
        if (is_for_loop(lines[i].line_text, lines[i].line_length) ||
            is_while_loop(lines[i].line_text, lines[i].line_length) ||
            strstr(lines[i].line_text, "if") ||
            strstr(lines[i].line_text, "else if") ||
            strstr(lines[i].line_text, "case") ||
            strstr(lines[i].line_text, "default")) {
            complexity++;
        }
    }
    return complexity;
}

int is_for_loop(char line[], int line_length) {
    return strstr(line, "for") != NULL;
}

int is_while_loop(char line[], int line_length) {
    return strstr(line, "while") != NULL;
}

int is_valid_function_syntax(char *line) {
    // Simplified check for function declaration/definition
    return strstr(line, "(") && strstr(line, ")");
}

int is_valid_variable_declaration(char *line) {
    // Simplified check for variable declaration
    return strstr(line, "int") || strstr(line, "float") || strstr(line, "char") || strstr(line, "double");
}
