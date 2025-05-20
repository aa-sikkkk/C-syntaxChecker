// Author: Aas1kkk
// Date: 2025-05-20
// Description: A GUI tool designed to analyze and validate the syntax of C and C++ codebases. It ensures code quality by detecting common syntax errors and providing detailed reports.
// File version: 1.2
// Last Update: 2024-07-17
// License: GNU License
// Recent changes: Added support for multiple file inputs, C++ specific constructs, improved output format, and added cyclomatic complexity calculation.

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <gtksourceview/gtksource.h>


// Structure to store each line of the file along with its line number and length
typedef struct {
    int line_number;
    int line_length;
    char line_text[1024];
} FileLine;

// Function declarations
void analyze_file(const char *input_filename, const char *output_dir);
void print_lines(FileLine lines[], int total_lines, FILE *output_file);
int find_comment_position(const char line[], int line_length);
void check_brackets(FileLine lines[], int total_lines, FILE *output_file);
void check_keywords(FileLine lines[], int total_lines, FILE *output_file, int is_cpp);
void count_functions_and_prototypes(FileLine lines[], int total_lines, FILE *output_file);
void check_keyword_usage(FileLine lines[], int total_lines, FILE *output_file, int is_cpp);
void check_builtin_functions(FileLine lines[], int total_lines, FILE *output_file, int is_cpp);
void check_print_scan_functions(FileLine lines[], int total_lines, FILE *output_file);
int is_print_function(const char line[], int line_length);
int is_scan_function(const char line[], int line_length);
void count_variables(FileLine lines[], int total_lines, FILE *output_file);
void check_file_operations(FileLine lines[], int total_lines, FILE *output_file);
int is_for_loop(const char *line, int length);
int is_while_loop(const char *line, int length);
int is_valid_function_syntax(const char *line);
int is_valid_variable_declaration(const char *line);
void check_semicolons(FileLine lines[], int total_lines, FILE *output_file);
void check_cpp_specific_constructs(FileLine lines[], int total_lines, FILE *output_file);
int calculate_cyclomatic_complexity(FileLine lines[], int total_lines);
void check_memory_leaks(FileLine lines[], int total_lines, FILE *output_file);
void check_pointer_usage(FileLine lines[], int total_lines, FILE *output_file);
void check_array_bounds(FileLine lines[], int total_lines, FILE *output_file);
void check_type_safety(FileLine lines[], int total_lines, FILE *output_file);
void check_naming_conventions(FileLine lines[], int total_lines, FILE *output_file);

// GTK UI components
GtkWidget *window;
GtkWidget *file_chooser_button;
GtkWidget *dir_chooser_button;
GtkWidget *analyze_button;
GtkWidget *text_view;
GtkTextBuffer *text_buffer;
GtkWidget *progress_bar;
GtkWidget *status_bar;
GtkWidget *source_view;
GtkSourceBuffer *source_buffer;
GtkWidget *notebook;
GtkWidget *results_view;
GtkTextBuffer *results_buffer;

// Callback for file chooser button click
void on_file_chooser_button_clicked(GtkWidget *widget, gpointer data) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(widget);
    char *filename = gtk_file_chooser_get_filename(chooser);
    if (filename) {
        // Set the selected file to the file chooser
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(file_chooser_button), filename);
        g_free(filename);
    }
}

// Function to handle directory selection
void on_dir_chooser_button_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Select Output Directory",
                                         GTK_WINDOW(window),
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Select", GTK_RESPONSE_ACCEPT,
                                         NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *foldername = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dir_chooser_button), foldername);
        g_free(foldername);
    }

    gtk_widget_destroy(dialog);
}

// Function to update status bar
void update_status(const char *message) {
    gtk_statusbar_pop(GTK_STATUSBAR(status_bar), 0);
    gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, message);
}

// Function to update progress bar
void update_progress(double fraction) {
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), fraction);
}

// Function to display analysis results in the GUI
void display_results(const char *filename, const char *results) {
    GtkTextIter iter;
    char *header = g_strdup_printf("\n=== Analysis Results for %s ===\n\n", filename);
    
    gtk_text_buffer_get_end_iter(results_buffer, &iter);
    gtk_text_buffer_insert(results_buffer, &iter, header, -1);
    gtk_text_buffer_insert(results_buffer, &iter, results, -1);
    gtk_text_buffer_insert(results_buffer, &iter, "\n", -1);
    
    g_free(header);
}

// Function to read file content and display in source view
void display_source_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        update_status("Error: Could not open file for display");
        return;
    }

    char buffer[1024];
    GString *content = g_string_new("");
    
    while (fgets(buffer, sizeof(buffer), file)) {
        g_string_append(content, buffer);
    }
    
    gtk_source_buffer_set_text(source_buffer, content->str, -1);
    g_string_free(content, TRUE);
    fclose(file);
}

// Modified analyze button callback
void on_analyze_button_clicked(GtkWidget *widget, gpointer data) {
    // Get selected files from file chooser button
    GtkFileChooser *file_chooser = GTK_FILE_CHOOSER(file_chooser_button);
    GSList *filenames = gtk_file_chooser_get_filenames(file_chooser);
    
    // Get selected directory from directory chooser button
    GtkFileChooser *dir_chooser = GTK_FILE_CHOOSER(dir_chooser_button);
    char *output_dir = gtk_file_chooser_get_filename(dir_chooser);

    if (!output_dir) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_CLOSE,
            "No output directory selected!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // Clear previous results
    gtk_text_buffer_set_text(results_buffer, "", -1);
    
    // Count total files for progress calculation
    int total_files = g_slist_length(filenames);
    int current_file = 0;

    // Process each selected file
    for (GSList *iter = filenames; iter != NULL; iter = iter->next) {
        char *filename = (char *)iter->data;
        current_file++;
        
        // Update status and progress
        char *status = g_strdup_printf("Analyzing file %d of %d: %s", 
                                     current_file, total_files, filename);
        update_status(status);
        update_progress((double)current_file / total_files);
        g_free(status);

        // Display source file
        display_source_file(filename);

        // Create temporary buffer for results
        GString *results = g_string_new("");
        FILE *temp_file = tmpfile();
        
        // Read file lines and analyze
        int total_lines;
        FileLine *lines = read_file_lines(filename, &total_lines);
        if (lines) {
            // Perform syntax analysis
            perform_syntax_analysis(lines, total_lines, temp_file);
            free(lines);
        }

        // Read results from temporary file
        rewind(temp_file);
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), temp_file)) {
            g_string_append(results, buffer);
        }
        fclose(temp_file);

        // Display results in GUI
        display_results(filename, results->str);
        g_string_free(results, TRUE);

        g_free(filename);
    }
    g_slist_free(filenames);
    g_free(output_dir);

    update_status("Analysis completed");
    update_progress(1.0);
}

// Function to create menu bar
GtkWidget* create_menu_bar(void) {
    GtkWidget *menu_bar = gtk_menu_bar_new();
    
    // File menu
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    
    GtkWidget *open_item = gtk_menu_item_new_with_label("Open");
    g_signal_connect(open_item, "activate", 
                    G_CALLBACK(on_file_chooser_button_clicked), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_item);
    
    GtkWidget *quit_item = gtk_menu_item_new_with_label("Quit");
    g_signal_connect(quit_item, "activate", G_CALLBACK(gtk_main_quit), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);
    
    return menu_bar;
}

// Function to analyze a single file
void analyze_file(const char *input_filename, const char *output_dir) {
    FILE *input_file;
    FileLine lines[100];
    char buffer[1024];
    int total_lines = 0, line_length, comment_position;
    int is_cpp = 0;

    // Determine file type based on extension
    if (strstr(input_filename, ".cpp") != NULL) {
        is_cpp = 1;
    } else if (strstr(input_filename, ".c") == NULL) {
        g_print("Error: Unsupported file extension for file %s. Please use .c or .cpp files.\n", input_filename);
        return;
    }

    input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        g_print("Error: Could not open input file %s.\n", input_filename);
        return;
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

    // Create output file path
    char output_filename[1024];
    snprintf(output_filename, sizeof(output_filename), "%s/%s_output.txt", output_dir, g_path_get_basename(input_filename));
    
    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        g_print("Error: Could not create output file %s.\n", output_filename);
        return;
    }

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

    // --- Call advanced checks ---
    check_memory_leaks(lines, total_lines, output_file);
    check_pointer_usage(lines, total_lines, output_file);
    check_array_bounds(lines, total_lines, output_file);
    check_type_safety(lines, total_lines, output_file);
    check_naming_conventions(lines, total_lines, output_file);
    // --- End advanced checks ---

    fclose(output_file);
}

// Function to print the lines
void print_lines(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        fprintf(output_file, "Line %d: %s", lines[i].line_number, lines[i].line_text);
    }
}

// Function to find the position of a comment in a line
int find_comment_position(const char line[], int line_length) {
    for (int i = 0; i < line_length - 1; i++) {
        if (line[i] == '/' && line[i + 1] == '/') {
            return i;
        }
    }
    return -1;
}

// Function to check the presence of matching brackets
void check_brackets(FileLine lines[], int total_lines, FILE *output_file) {
    int bracket_count = 0;

    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < lines[i].line_length; j++) {
            if (lines[i].line_text[j] == '{') {
                bracket_count++;
            } else if (lines[i].line_text[j] == '}') {
                bracket_count--;
            }
        }
    }

    if (bracket_count != 0) {
        fprintf(output_file, "Error: Mismatched brackets detected. Check for missing '{' or '}'.\n");
    } else {
        fprintf(output_file, "Brackets check passed.\n");
    }
}

// Function to check for the presence of keywords
void check_keywords(FileLine lines[], int total_lines, FILE *output_file, int is_cpp) {
    // List of C/C++ keywords to check
    const char *keywords[] = {
        "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
        "else", "enum", "extern", "float", "for", "goto", "if", "int", "long",
        "register", "return", "short", "signed", "sizeof", "static", "struct",
        "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
    };
    const char *cpp_keywords[] = {
        "alignas", "alignof", "and", "and_eq", "asm", "bitand", "bitor", "bool",
        "catch", "class", "compl", "constexpr", "const_cast", "decltype", "delete",
        "dynamic_cast", "explicit", "export", "false", "friend", "inline", "mutable",
        "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or",
        "or_eq", "private", "protected", "public", "reinterpret_cast", "static_assert",
        "static_cast", "template", "this", "thread_local", "throw", "true", "try", "typeid",
        "typename", "using", "virtual", "wchar_t", "xor", "xor_eq"
    };

    int keyword_count = sizeof(keywords) / sizeof(keywords[0]);
    int cpp_keyword_count = sizeof(cpp_keywords) / sizeof(cpp_keywords[0]);

    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < keyword_count; j++) {
            if (strstr(lines[i].line_text, keywords[j]) != NULL) {
                fprintf(output_file, "Keyword found: %s at line %d\n", keywords[j], lines[i].line_number);
            }
        }
        if (is_cpp) {
            for (int j = 0; j < cpp_keyword_count; j++) {
                if (strstr(lines[i].line_text, cpp_keywords[j]) != NULL) {
                    fprintf(output_file, "C++ keyword found: %s at line %d\n", cpp_keywords[j], lines[i].line_number);
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
            if (strstr(lines[i].line_text, ";") != NULL) {
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
    // Example keyword usage check (can be expanded with more checks)
    for (int i = 0; i < total_lines; i++) {
        if (strstr(lines[i].line_text, "goto") != NULL) {
            fprintf(output_file, "Warning: 'goto' statement found at line %d\n", lines[i].line_number);
        }
    }
}

// Function to check for built-in functions
void check_builtin_functions(FileLine lines[], int total_lines, FILE *output_file, int is_cpp) {
    // List of common C built-in functions to check
    const char *c_builtins[] = {"malloc", "free", "printf", "scanf", "fopen", "fclose"};
    int c_builtin_count = sizeof(c_builtins) / sizeof(c_builtins[0]);

    // List of common C++ built-in functions to check
    const char *cpp_builtins[] = {"std::cout", "std::cin", "std::endl", "new", "delete"};
    int cpp_builtin_count = sizeof(cpp_builtins) / sizeof(cpp_builtins[0]);

    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < c_builtin_count; j++) {
            if (strstr(lines[i].line_text, c_builtins[j]) != NULL) {
                fprintf(output_file, "Built-in function found: %s at line %d\n", c_builtins[j], lines[i].line_number);
            }
        }
        if (is_cpp) {
            for (int j = 0; j < cpp_builtin_count; j++) {
                if (strstr(lines[i].line_text, cpp_builtins[j]) != NULL) {
                    fprintf(output_file, "C++ built-in function found: %s at line %d\n", cpp_builtins[j], lines[i].line_number);
                }
            }
        }
    }
}

// Function to check for the use of printf and scanf functions
void check_print_scan_functions(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (is_print_function(lines[i].line_text, lines[i].line_length)) {
            fprintf(output_file, "printf function found at line %d\n", lines[i].line_number);
        }
        if (is_scan_function(lines[i].line_text, lines[i].line_length)) {
            fprintf(output_file, "scanf function found at line %d\n", lines[i].line_number);
        }
    }
}

// Function to count the number of variables
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
    // List of common file operation functions to check
    const char *file_operations[] = {"fopen", "fclose", "fread", "fwrite", "fprintf", "fscanf"};
    int file_operation_count = sizeof(file_operations) / sizeof(file_operations[0]);

    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < file_operation_count; j++) {
            if (strstr(lines[i].line_text, file_operations[j]) != NULL) {
                fprintf(output_file, "File operation function found: %s at line %d\n", file_operations[j], lines[i].line_number);
            }
        }
    }
}

// Function to check if a line is a for loop
int is_for_loop(const char *line, int length) {
    return strstr(line, "for") != NULL;
}

// Function to check if a line is a while loop
int is_while_loop(const char *line, int length) {
    return strstr(line, "while") != NULL;
}

// Function to check if a line contains valid function syntax
int is_valid_function_syntax(const char *line) {
    return strstr(line, "(") != NULL && strstr(line, ")") != NULL;
}

// Function to check if a line contains a valid variable declaration
int is_valid_variable_declaration(const char *line) {
    // Basic variable declaration check
    return strstr(line, "int ") != NULL || strstr(line, "char ") != NULL ||
           strstr(line, "float ") != NULL || strstr(line, "double ") != NULL;
}

// Function to check for the presence of semicolons at the end of lines
void check_semicolons(FileLine lines[], int total_lines, FILE *output_file) {
    for (int i = 0; i < total_lines; i++) {
        if (lines[i].line_text[lines[i].line_length - 2] != ';' && 
            lines[i].line_text[lines[i].line_length - 1] != '{' && 
            lines[i].line_text[lines[i].line_length - 1] != '}') {
            fprintf(output_file, "Warning: Missing semicolon at line %d\n", lines[i].line_number);
        }
    }
}

// Function to check C++ specific constructs
void check_cpp_specific_constructs(FileLine lines[], int total_lines, FILE *output_file) {
    // List of common C++ specific constructs to check
    const char *cpp_constructs[] = {"class ", "public:", "private:", "protected:", "namespace ", 
                                    "std::", "new ", "delete ", "virtual ", "template<", 
                                    "constexpr ", "override", "nullptr"};
    int cpp_construct_count = sizeof(cpp_constructs) / sizeof(cpp_constructs[0]);

    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < cpp_construct_count; j++) {
            if (strstr(lines[i].line_text, cpp_constructs[j]) != NULL) {
                fprintf(output_file, "C++ specific construct found: %s at line %d\n", cpp_constructs[j], lines[i].line_number);
            }
        }
    }
}

// Function to calculate cyclomatic complexity
int calculate_cyclomatic_complexity(FileLine lines[], int total_lines) {
    int complexity = 1; // Start with 1 for the method itself

    for (int i = 0; i < total_lines; i++) {
        if (is_for_loop(lines[i].line_text, lines[i].line_length) ||
            is_while_loop(lines[i].line_text, lines[i].line_length) ||
            strstr(lines[i].line_text, "if") != NULL ||
            strstr(lines[i].line_text, "else if") != NULL ||
            strstr(lines[i].line_text, "case") != NULL ||
            strstr(lines[i].line_text, "default") != NULL ||
            strstr(lines[i].line_text, "switch") != NULL ||
            strstr(lines[i].line_text, "catch") != NULL) {
            complexity++;
        }
    }

    return complexity;
}

// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    gtk_source_init();

    // Create window and widgets
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Advanced Code Syntax Analyzer");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create vertical box for main layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Add menu bar
    GtkWidget *menu_bar = create_menu_bar();
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

    // Create horizontal box for file selection
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    file_chooser_button = gtk_file_chooser_button_new("Select Files", 
                                                     GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser_button), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), file_chooser_button, TRUE, TRUE, 0);

    dir_chooser_button = gtk_file_chooser_button_new("Select Output Directory", 
                                                    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_box_pack_start(GTK_BOX(hbox), dir_chooser_button, TRUE, TRUE, 0);

    analyze_button = gtk_button_new_with_label("Analyze");
    gtk_box_pack_start(GTK_BOX(hbox), analyze_button, FALSE, FALSE, 0);
    g_signal_connect(analyze_button, "clicked", G_CALLBACK(on_analyze_button_clicked), NULL);

    // Create notebook for source and results
    notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

    // Source view page
    GtkWidget *source_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    source_buffer = gtk_source_buffer_new(NULL);
    source_view = gtk_source_view_new_with_buffer(source_buffer);
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(source_view), TRUE);
    gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(source_view), TRUE);
    gtk_box_pack_start(GTK_BOX(source_page), source_view, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), source_page, 
                            gtk_label_new("Source Code"));

    // Results page
    GtkWidget *results_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    results_buffer = gtk_text_buffer_new(NULL);
    results_view = gtk_text_view_new_with_buffer(results_buffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(results_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(results_view), FALSE);
    gtk_box_pack_start(GTK_BOX(results_page), results_view, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), results_page, 
                            gtk_label_new("Analysis Results"));

    // Progress bar
    progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), progress_bar, FALSE, FALSE, 0);

    // Status bar
    status_bar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(vbox), status_bar, FALSE, FALSE, 0);
    update_status("Ready");

    // Connect the window destroy event
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    gtk_source_finalize();
    return 0;
}
