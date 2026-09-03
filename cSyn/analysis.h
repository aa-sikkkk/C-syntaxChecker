#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "lexer.h"
#include <stdio.h>

// Severity levels
typedef enum {
    SEV_ERROR, SEV_WARNING, SEV_INFO, SEV_STYLE
} Severity;

// Diagnostic message
typedef struct {
    Severity severity;
    int line;
    int column;
    char message[512];
} Diagnostic;

// Diagnostics collection
typedef struct {
    Diagnostic *items;
    int count;
    int capacity;
} DiagnosticList;

void diag_init(DiagnosticList *list);
void diag_add(DiagnosticList *list, Severity sev, int line, int column, const char *fmt, ...);
void diag_free(DiagnosticList *list);

// Token stream navigation helpers
int find_matching_brace(TokenStream *stream, int open_index);
int find_matching_paren(TokenStream *stream, int open_index);
int find_matching_bracket(TokenStream *stream, int open_index);
int find_next_semicolon(TokenStream *stream, int from);
int find_next_statement_start(TokenStream *stream, int from);
bool token_at(TokenStream *stream, int index, TokenType type);
int skip_whitespace_and_comments(TokenStream *stream, int from);

// Analysis functions (token-based)
void analyze_brackets(TokenStream *stream, DiagnosticList *diags);
void analyze_semicolons(TokenStream *stream, DiagnosticList *diags);
void analyze_keywords(TokenStream *stream, DiagnosticList *diags, int is_cpp);
void analyze_functions(TokenStream *stream, DiagnosticList *diags);
void analyze_variables(TokenStream *stream, DiagnosticList *diags);
void analyze_keyword_usage(TokenStream *stream, DiagnosticList *diags);
void analyze_memory_ops(TokenStream *stream, DiagnosticList *diags);
void analyze_io_ops(TokenStream *stream, DiagnosticList *diags);
void analyze_file_ops(TokenStream *stream, DiagnosticList *diags);
void analyze_cpp_constructs(TokenStream *stream, DiagnosticList *diags);
int  calculate_complexity(TokenStream *stream);
void analyze_memory_leaks(TokenStream *stream, DiagnosticList *diags);
void analyze_goto(TokenStream *stream, DiagnosticList *diags);

// Advanced analyses (Phase 4)
void analyze_unused_variables(TokenStream *stream, DiagnosticList *diags);
void analyze_uninitialized_variables(TokenStream *stream, DiagnosticList *diags);
void analyze_shadowed_variables(TokenStream *stream, DiagnosticList *diags);
void analyze_null_pointer_deref(TokenStream *stream, DiagnosticList *diags);
void analyze_buffer_overflow(TokenStream *stream, DiagnosticList *diags);
void analyze_format_strings(TokenStream *stream, DiagnosticList *diags);
void analyze_switch_fallthrough(TokenStream *stream, DiagnosticList *diags);
void analyze_dead_code(TokenStream *stream, DiagnosticList *diags);
void analyze_nesting_depth(TokenStream *stream, DiagnosticList *diags);

// Output
void print_diagnostics(DiagnosticList *diags, FILE *output);
void print_file_lines(const char *source, FILE *output);

#endif
