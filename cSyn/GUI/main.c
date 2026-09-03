// Author: Aas1kkk
// Date: 2025-05-20
// Description: A GUI tool to analyze and validate the syntax of C and C++ codebases.
// Uses the shared lexer + analysis library (lexer.c / analysis.c).
// License: GNU License

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lexer.h"
#include "../analysis.h"

#if defined(HAVE_GTKSOURCEVIEW3) || defined(HAVE_GTKSOURCEVIEW4)
#define HAVE_GTKSOURCEVIEW
#include <gtksourceview/gtksource.h>
#endif

// GTK UI components
GtkWidget *window;
GtkWidget *file_chooser_button;
GtkWidget *dir_chooser_button;
GtkWidget *analyze_button;
GtkWidget *progress_bar;
GtkWidget *status_bar;
#ifndef HAVE_GTKSOURCEVIEW
static GtkWidget *source_view;
static GtkTextBuffer *source_buffer;
#else
GtkWidget *source_view;
GtkSourceBuffer *source_buffer;
#endif
GtkWidget *notebook;
GtkTextBuffer *results_buffer;

// Callback for file chooser button click
void on_file_chooser_button_clicked(GtkWidget *widget, gpointer data) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(widget);
    char *filename = gtk_file_chooser_get_filename(chooser);
    if (filename) {
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

// Read entire file into a null-terminated string (NULL on failure)
static char *read_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) { fclose(f); return NULL; }

    char *buf = malloc(size + 1);
    if (!buf) { fclose(f); return NULL; }

    size_t read = fread(buf, 1, size, f);
    buf[read] = '\0';
    fclose(f);
    return buf;
}

static int is_cpp_file(const char *filename) {
    size_t len = strlen(filename);
    if (len >= 4 && strcmp(filename + len - 4, ".cpp") == 0) return 1;
    if (len >= 5 && strcmp(filename + len - 5, ".cxx") == 0) return 1;
    if (len >= 5 && strcmp(filename + len - 5, ".cc") == 0) return 1;
    if (len >= 4 && strcmp(filename + len - 4, ".hpp") == 0) return 1;
    if (len >= 5 && strcmp(filename + len - 5, ".hxx") == 0) return 1;
    if (len >= 3 && strcmp(filename + len - 3, ".hh") == 0) return 1;
    return 0;
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

    char buffer[8192];
    GString *content = g_string_new("");

    while (fgets(buffer, sizeof(buffer), file)) {
        g_string_append(content, buffer);
    }

#ifdef HAVE_GTKSOURCEVIEW
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(source_buffer), content->str, -1);
#ifdef HAVE_GTKSOURCEVIEW4
    // v4: language is applied via the buffer's language id
    GtkSourceLanguage *lang = gtk_source_language_manager_get_language(
        gtk_source_language_manager_get_default(),
        is_cpp_file(filename) ? "cpp" : "c");
    gtk_source_buffer_set_language(source_buffer, lang);
    gtk_source_buffer_set_highlight_syntax(source_buffer, TRUE);
#endif
#else
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(source_buffer), content->str, -1);
#endif
    g_string_free(content, TRUE);
    fclose(file);
}

// Modified analyze button callback
void on_analyze_button_clicked(GtkWidget *widget, gpointer data) {
    GtkFileChooser *file_chooser = GTK_FILE_CHOOSER(file_chooser_button);
    GSList *filenames = gtk_file_chooser_get_filenames(file_chooser);

    // Clear previous results
    gtk_text_buffer_set_text(results_buffer, "", -1);

    int total_files = g_slist_length(filenames);
    int current_file = 0;

    for (GSList *iter = filenames; iter != NULL; iter = iter->next) {
        char *filename = (char *)iter->data;
        current_file++;

        char *status = g_strdup_printf("Analyzing file %d of %d: %s",
                                       current_file, total_files, filename);
        update_status(status);
        update_progress((double)current_file / total_files);
        g_free(status);

        display_source_file(filename);

        char *source = read_file(filename);
        if (!source) {
            char msg[512];
            snprintf(msg, sizeof(msg), "Error: Could not read file %s\n", filename);
            display_results(filename, msg);
            g_free(filename);
            continue;
        }

        TokenStream stream = tokenize_source(source);
        if (!stream.tokens) {
            display_results(filename, "Error: Tokenization failed\n");
            free(source);
            g_free(filename);
            continue;
        }

        int is_cpp = is_cpp_file(filename);
        DiagnosticList diags;
        diag_init(&diags);

        analyze_brackets(&stream, &diags);
        analyze_semicolons(&stream, &diags);
        analyze_keywords(&stream, &diags, is_cpp);
        analyze_functions(&stream, &diags);
        analyze_variables(&stream, &diags);
        analyze_keyword_usage(&stream, &diags);
        analyze_memory_ops(&stream, &diags);
        analyze_io_ops(&stream, &diags);
        analyze_file_ops(&stream, &diags);
        if (is_cpp) analyze_cpp_constructs(&stream, &diags);

        int complexity = calculate_complexity(&stream);
        diag_add(&diags, SEV_INFO, 0, 0, "Cyclomatic complexity: %d", complexity);

        analyze_memory_leaks(&stream, &diags);

        // Render diagnostics into a string
        GString *results = g_string_new("");
        const char *sev_names[] = {"Error", "Warning", "Info", "Style"};
        int err = 0, war = 0, inf = 0, stl = 0;
        for (int i = 0; i < diags.count; i++) {
            Diagnostic *d = &diags.items[i];
            g_string_append_printf(results, "%s at line %d, col %d: %s\n",
                                   sev_names[d->severity], d->line, d->column, d->message);
            switch (d->severity) {
                case SEV_ERROR: err++; break;
                case SEV_WARNING: war++; break;
                case SEV_INFO: inf++; break;
                case SEV_STYLE: stl++; break;
            }
        }
        g_string_append_printf(results, "\n--- Summary ---\n");
        g_string_append_printf(results, "Errors: %d  Warnings: %d  Info: %d  Style: %d\n",
                               err, war, inf, stl);

        display_results(filename, results->str);

        g_string_free(results, TRUE);
        diag_free(&diags);
        token_stream_free(&stream);
        free(source);
        g_free(filename);
    }
    g_slist_free(filenames);

    update_status("Analysis completed");
    update_progress(1.0);
}

// Function to create menu bar
GtkWidget* create_menu_bar(void) {
    GtkWidget *menu_bar = gtk_menu_bar_new();

    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);

    GtkWidget *quit_item = gtk_menu_item_new_with_label("Quit");
    g_signal_connect(quit_item, "activate", G_CALLBACK(gtk_main_quit), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);

    return menu_bar;
}

// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
#ifdef HAVE_GTKSOURCEVIEW
    gtk_source_init();
#endif

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Advanced Code Syntax Analyzer");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *menu_bar = create_menu_bar();
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    file_chooser_button = gtk_file_chooser_button_new("Select Files",
                                                      GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser_button), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), file_chooser_button, TRUE, TRUE, 0);

    analyze_button = gtk_button_new_with_label("Analyze");
    gtk_box_pack_start(GTK_BOX(hbox), analyze_button, FALSE, FALSE, 0);
    g_signal_connect(analyze_button, "clicked", G_CALLBACK(on_analyze_button_clicked), NULL);

    notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

    // Source view page
    GtkWidget *source_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
#ifdef HAVE_GTKSOURCEVIEW
    source_buffer = gtk_source_buffer_new(NULL);
    source_view = gtk_source_view_new_with_buffer(GTK_SOURCE_BUFFER(source_buffer));
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(source_view), TRUE);
    gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(source_view), TRUE);
#else
    source_buffer = gtk_text_buffer_new(NULL);
    source_view = gtk_text_view_new_with_buffer(GTK_TEXT_BUFFER(source_buffer));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(source_view), FALSE);
#endif
    gtk_box_pack_start(GTK_BOX(source_page), source_view, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), source_page,
                            gtk_label_new("Source Code"));

    // Results page
    GtkWidget *results_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    results_buffer = gtk_text_buffer_new(NULL);
    GtkWidget *results_view = gtk_text_view_new_with_buffer(results_buffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(results_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(results_view), FALSE);
    gtk_box_pack_start(GTK_BOX(results_page), results_view, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), results_page,
                            gtk_label_new("Analysis Results"));

    progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), progress_bar, FALSE, FALSE, 0);

    status_bar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(vbox), status_bar, FALSE, FALSE, 0);
    update_status("Ready");

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

#ifdef HAVE_GTKSOURCEVIEW
    gtk_source_finalize();
#endif
    return 0;
}