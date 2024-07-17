#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024

// Structure to store each line of the file along with its line number and length
typedef struct {
    int line_number;
    int line_length;
    char line_text[MAX_LINE_LENGTH];
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
int is_valid_function_syntax(char *line);
int is_valid_variable_declaration(char *line);

// Global variables for GUI
GtkWidget *input_file_entry;
GtkWidget *output_file_entry;
GtkWidget *text_view;

// Function to load and analyze the file
void on_analyze_clicked(GtkWidget *widget, gpointer data) {
    const char *input_file_path = gtk_entry_get_text(GTK_ENTRY(input_file_entry));
    const char *output_file_path = gtk_entry_get_text(GTK_ENTRY(output_file_entry));

    FILE *input_file;
    FILE *output_file;
    FileLine lines[1000];
    char buffer[MAX_LINE_LENGTH];
    int total_lines = 0, i = 0, line_length, comment_position;

    input_file = fopen(input_file_path, "r");
    if (input_file == NULL) {
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)), "Error: Could not open input file.\n", -1);
        return;
    }

    output_file = fopen(output_file_path, "w");
    if (output_file == NULL) {
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)), "Error: Could not open output file.\n", -1);
        fclose(input_file);
        return;
    }

    // Read lines from the input file
    while (fgets(buffer, MAX_LINE_LENGTH, input_file) != NULL) {
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

    fclose(output_file);

    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)), "Analysis complete. Results written to output file.\n", -1);
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

// Function to print the lines
void print_lines(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        fprintf(output_file, "Line %d: %s", lines[i].line_number, lines[i].line_text);
    }
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

// Function to check if a line contains a print function
int is_print_function(char line[], int line_length) {
    return strstr(line, "printf") != NULL || strstr(line, "fprintf") != NULL || strstr(line, "puts") != NULL;
}

// Function to check if a line contains a scan function
int is_scan_function(char line[], int line_length) {
    return strstr(line, "scanf") != NULL || strstr(line, "fscanf") != NULL || strstr(line, "gets") != NULL;
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

// Function to check if a line has valid function syntax
int is_valid_function_syntax(char *line) {
    char *paren_open = strchr(line, '(');
    char *paren_close = strchr(line, ')');
    return paren_open && paren_close && (paren_close > paren_open);
}

// Function to check if a line is a valid variable declaration
int is_valid_variable_declaration(char *line) {
    const char *valid_types[] = {"int", "float", "char", "double"};
    int type_count = sizeof(valid_types) / sizeof(valid_types[0]);
    for (int i = 0; i < type_count; i++) {
        if (strstr(line, valid_types[i]) == line) {
            return 1;
        }
    }
    return 0;
}

// Function to quit the application
void on_quit_clicked(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

// Main function to initialize the GTK application
int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label_input;
    GtkWidget *label_output;
    GtkWidget *analyze_button;
    GtkWidget *quit_button;
    GtkWidget *scroll_window;

    gtk_init(&argc, &argv);

    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "C/C++ Syntax Checker");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create grid and attach it to the window
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Create and attach widgets to the grid
    label_input = gtk_label_new("Input File:");
    gtk_grid_attach(GTK_GRID(grid), label_input, 0, 0, 1, 1);
    input_file_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), input_file_entry, 1, 0, 1, 1);

    label_output = gtk_label_new("Output File:");
    gtk_grid_attach(GTK_GRID(grid), label_output, 0, 1, 1, 1);
    output_file_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), output_file_entry, 1, 1, 1, 1);

    analyze_button = gtk_button_new_with_label("Analyze");
    g_signal_connect(analyze_button, "clicked", G_CALLBACK(on_analyze_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), analyze_button, 0, 2, 1, 1);

    quit_button = gtk_button_new_with_label("Quit");
    g_signal_connect(quit_button, "clicked", G_CALLBACK(on_quit_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), quit_button, 1, 2, 1, 1);

    scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scroll_window, 600, 400);
    gtk_grid_attach(GTK_GRID(grid), scroll_window, 0, 3, 2, 1);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scroll_window), text_view);

    // Connect the destroy signal to quit the GTK main loop
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Show all widgets and enter the GTK main loop
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}


// gcc -o analyzer main.c $(pkg-config --cflags --libs gtk+-3.0)
