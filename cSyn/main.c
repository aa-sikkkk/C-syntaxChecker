// Author: Aas1kkk
// Date: 2025-05-20
// Description: C/C++ syntax checker with token-based static analysis.
// License: GNU License

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "analysis.h"
#include "report.h"

// Read entire file into a null-terminated string
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

// Run every analysis on the source and fill diags. Returns the raw source
// (caller frees) or NULL if the file could not be read/tokenized.
static char *analyze_file(const char *input_filename, FILE *output_file,
                          DiagnosticList *diags) {
    fprintf(output_file, "=== Analysis for: %s ===\n\n", input_filename);

    char *source = read_file(input_filename);
    if (!source) {
        fprintf(output_file, "Error: Could not read file '%s'\n\n", input_filename);
        return NULL;
    }

    // Print source with line numbers
    fprintf(output_file, "--- Source Code ---\n");
    print_file_lines(source, output_file);
    fprintf(output_file, "\n");

    // Tokenize
    TokenStream stream = tokenize_source(source);
    if (!stream.tokens) {
        fprintf(output_file, "Error: Tokenization failed\n\n");
        free(source);
        return NULL;
    }

    int is_cpp = is_cpp_file(input_filename);
    diag_init(diags);

    // Run all analyses
    analyze_brackets(&stream, diags);
    analyze_semicolons(&stream, diags);
    analyze_keywords(&stream, diags, is_cpp);
    analyze_functions(&stream, diags);
    analyze_variables(&stream, diags);
    analyze_keyword_usage(&stream, diags);
    analyze_memory_ops(&stream, diags);
    analyze_io_ops(&stream, diags);
    analyze_file_ops(&stream, diags);
    if (is_cpp) analyze_cpp_constructs(&stream, diags);

    int complexity = calculate_complexity(&stream);
    diag_add(diags, SEV_INFO, 0, 0, "Cyclomatic complexity: %d", complexity);

    analyze_memory_leaks(&stream, diags);

    // Advanced analyses
    analyze_unused_variables(&stream, diags);
    analyze_uninitialized_variables(&stream, diags);
    analyze_shadowed_variables(&stream, diags);
    analyze_null_pointer_deref(&stream, diags);
    analyze_buffer_overflow(&stream, diags);
    analyze_format_strings(&stream, diags);
    analyze_switch_fallthrough(&stream, diags);
    analyze_dead_code(&stream, diags);
    analyze_nesting_depth(&stream, diags);

    // Output results (text)
    fprintf(output_file, "--- Diagnostics ---\n\n");
    print_diagnostics(diags, output_file);
    fprintf(output_file, "\n");

    token_stream_free(&stream);
    return source;
}

static int report_applies(const char *arg, const char *flag) {
    return strcmp(arg, flag) == 0;
}

static void print_usage(const char *prog) {
    printf("Usage: %s [options] <source_file1> [source_file2] ...\n", prog);
    printf("C/C++ syntax checker with token-based static analysis.\n");
    printf("Options:\n");
    printf("  -h, --help   show this help\n");
    printf("  -html        also write an HTML report (output.html)\n");
    printf("  -json        also write a JSON report (output.json)\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    // Parse flags (may be interleaved with file names)
    int want_html = 0, want_json = 0;
    int file_start = 1;
    for (int i = 1; i < argc; i++) {
        if (report_applies(argv[i], "-h") || report_applies(argv[i], "--help")) {
            print_usage(argv[0]);
            return 0;
        }
        if (report_applies(argv[i], "-html")) { want_html = 1; continue; }
        if (report_applies(argv[i], "-json")) { want_json = 1; continue; }
        file_start = i;
        break;
    }

    FILE *output_file = fopen("output.txt", "w");
    if (!output_file) {
        fprintf(stderr, "Error: Could not open output file.\n");
        return 1;
    }

    FILE *html_file = want_html ? fopen("output.html", "w") : NULL;
    if (want_html && !html_file) {
        fprintf(stderr, "Error: Could not open output.html for writing.\n");
    }
    FILE *json_file = want_json ? fopen("output.json", "w") : NULL;
    if (want_json && !json_file) {
        fprintf(stderr, "Error: Could not open output.json for writing.\n");
    }

    for (int i = file_start; i < argc; i++) {
        if (report_applies(argv[i], "-html") || report_applies(argv[i], "-json") ||
            report_applies(argv[i], "-h") || report_applies(argv[i], "--help")) continue;
        DiagnosticList diags;
        char *source = analyze_file(argv[i], output_file, &diags);
        if (html_file)
            generate_html_report(argv[i], source ? source : "", &diags, html_file);
        if (json_file)
            generate_json_report(argv[i], source ? source : "", &diags, json_file);
        diag_free(&diags);
        free(source);
    }

    fclose(output_file);
    if (html_file) fclose(html_file);
    if (json_file) fclose(json_file);

    printf("Analysis complete. Results written to output.txt");
    if (html_file) printf(" and output.html");
    if (json_file) printf(" and output.json");
    printf("\n");
    return 0;
}
