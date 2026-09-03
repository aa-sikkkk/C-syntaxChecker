#include "analysis.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// ---- Diagnostics ----

void diag_init(DiagnosticList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void diag_add(DiagnosticList *list, Severity sev, int line, int column, const char *fmt, ...) {
    if (list->count >= list->capacity) {
        list->capacity = list->capacity ? list->capacity * 2 : 256;
        list->items = realloc(list->items, list->capacity * sizeof(Diagnostic));
    }
    Diagnostic *d = &list->items[list->count++];
    d->severity = sev;
    d->line = line;
    d->column = column;
    va_list args;
    va_start(args, fmt);
    vsnprintf(d->message, sizeof(d->message), fmt, args);
    va_end(args);
}

void diag_free(DiagnosticList *list) {
    if (list->items) free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

// ---- Token stream navigation ----

int skip_whitespace_and_comments(TokenStream *stream, int from) {
    while (from < stream->count) {
        Token t = stream->tokens[from];
        if (t.type == TOK_WHITESPACE || t.type == TOK_NEWLINE ||
            t.type == TOK_COMMENT || t.type == TOK_BLOCK_COMMENT) {
            from++;
        } else break;
    }
    return from;
}

int find_matching_brace(TokenStream *stream, int open_index) {
    int depth = 0;
    for (int i = open_index; i < stream->count; i++) {
        if (stream->tokens[i].type == TOK_LBRACE) depth++;
        else if (stream->tokens[i].type == TOK_RBRACE) {
            depth--;
            if (depth == 0) return i;
        }
    }
    return -1;
}

int find_matching_paren(TokenStream *stream, int open_index) {
    int depth = 0;
    for (int i = open_index; i < stream->count; i++) {
        if (stream->tokens[i].type == TOK_LPAREN) depth++;
        else if (stream->tokens[i].type == TOK_RPAREN) {
            depth--;
            if (depth == 0) return i;
        }
    }
    return -1;
}

int find_matching_bracket(TokenStream *stream, int open_index) {
    int depth = 0;
    for (int i = open_index; i < stream->count; i++) {
        if (stream->tokens[i].type == TOK_LBRACKET) depth++;
        else if (stream->tokens[i].type == TOK_RBRACKET) {
            depth--;
            if (depth == 0) return i;
        }
    }
    return -1;
}

int find_next_semicolon(TokenStream *stream, int from) {
    for (int i = from; i < stream->count; i++) {
        if (stream->tokens[i].type == TOK_SEMICOLON) return i;
        if (stream->tokens[i].type == TOK_LBRACE) return -1; // hit block before semicolon
    }
    return -1;
}

int find_next_statement_start(TokenStream *stream, int from) {
    for (int i = from; i < stream->count; i++) {
        TokenType t = stream->tokens[i].type;
        if (t != TOK_WHITESPACE && t != TOK_NEWLINE && t != TOK_COMMENT &&
            t != TOK_BLOCK_COMMENT && t != TOK_SEMICOLON && t != TOK_LBRACE)
            return i;
    }
    return -1;
}

bool token_at(TokenStream *stream, int index, TokenType type) {
    if (index < 0 || index >= stream->count) return false;
    return stream->tokens[index].type == type;
}

// ---- Analysis: Bracket matching ----

void analyze_brackets(TokenStream *stream, DiagnosticList *diags) {
    int *stack = NULL;
    int stack_cap = 0;
    int stack_top = -1;

    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type == TOK_LBRACE || t.type == TOK_LPAREN || t.type == TOK_LBRACKET) {
            if (stack_top >= stack_cap - 1) {
                stack_cap = stack_cap ? stack_cap * 2 : 128;
                stack = realloc(stack, stack_cap * sizeof(int));
            }
            stack[++stack_top] = i;
        }
        else if (t.type == TOK_RBRACE || t.type == TOK_RPAREN || t.type == TOK_RBRACKET) {
            if (stack_top < 0) {
                diag_add(diags, SEV_ERROR, t.line, t.column,
                    "Unmatched closing '%s'", token_type_name(t.type));
                continue;
            }

            int open_idx = stack[stack_top];
            Token open = stream->tokens[open_idx];
            bool matched =
                (open.type == TOK_LBRACE && t.type == TOK_RBRACE) ||
                (open.type == TOK_LPAREN && t.type == TOK_RPAREN) ||
                (open.type == TOK_LBRACKET && t.type == TOK_RBRACKET);

            if (matched) {
                stack_top--;
                // Warn on large gaps
                if (t.line - open.line > 100) {
                    diag_add(diags, SEV_WARNING, t.line, t.column,
                        "Large gap between matching brackets (lines %d-%d)",
                        open.line, t.line);
                }
                // Warn on empty blocks
                if (open.type == TOK_LBRACE && t.type == TOK_RBRACE) {
                    // Check if there's anything between { and }
                    bool empty = true;
                    for (int j = open_idx + 1; j < i; j++) {
                        TokenType ct = stream->tokens[j].type;
                        if (ct != TOK_WHITESPACE && ct != TOK_NEWLINE &&
                            ct != TOK_COMMENT && ct != TOK_BLOCK_COMMENT) {
                            empty = false;
                            break;
                        }
                    }
                    if (empty) {
                        diag_add(diags, SEV_WARNING, open.line, open.column,
                            "Empty block");
                    }
                }
            } else {
                diag_add(diags, SEV_ERROR, t.line, t.column,
                    "Mismatched bracket: '%s' at line %d closed by '%s'",
                    token_type_name(open.type), open.line, token_type_name(t.type));
                stack_top--;
            }
        }
    }

    // Remaining unclosed brackets
    while (stack_top >= 0) {
        Token open = stream->tokens[stack[stack_top--]];
        diag_add(diags, SEV_ERROR, open.line, open.column,
            "Unclosed '%s'", token_type_name(open.type));
    }

    free(stack);
}

// ---- Analysis: Semicolons ----

void analyze_semicolons(TokenStream *stream, DiagnosticList *diags) {
    // States: nothing, had_value, had_assign
    // Missing semicolons: statement ends without ; before next statement or }
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];

        // Skip preprocessor, comments, whitespace
        if (t.type == TOK_PREPROCESSOR || t.type == TOK_INCLUDE || t.type == TOK_DEFINE ||
            t.type == TOK_IFDEF || t.type == TOK_IFNDEF || t.type == TOK_ENDIF ||
            t.type == TOK_ELSE_PP || t.type == TOK_PRAGMA ||
            t.type == TOK_COMMENT || t.type == TOK_BLOCK_COMMENT ||
            t.type == TOK_WHITESPACE || t.type == TOK_NEWLINE ||
            t.type == TOK_EOF) continue;

        // Skip if we're inside control structure headers (for/if/while/switch)
        // These have their own parenthesized expressions
        if (t.type == TOK_KEYWORD) {
            // for, if, while, switch don't need semicolons after their parens
            if ((t.length == 3 && memcmp(t.start, "for", 3) == 0) ||
                (t.length == 2 && memcmp(t.start, "if", 2) == 0) ||
                (t.length == 5 && memcmp(t.start, "while", 5) == 0) ||
                (t.length == 6 && memcmp(t.start, "switch", 6) == 0)) {
                // Skip to matching brace or end of control structure
                // Find the opening brace that follows this statement
                int depth = 0;
                for (int j = i; j < stream->count; j++) {
                    if (stream->tokens[j].type == TOK_LBRACE) {
                        depth++;
                        if (depth == 1) { i = j; break; }
                    }
                    if (stream->tokens[j].type == TOK_RBRACE && depth > 0) {
                        depth--;
                    }
                }
                continue;
            }
            // return, break, continue, goto, throw must have semicolons
            // enum/struct/union/typedef declarations may not
            if ((t.length == 6 && memcmp(t.start, "return", 6) == 0) ||
                (t.length == 5 && memcmp(t.start, "break", 5) == 0) ||
                (t.length == 8 && memcmp(t.start, "continue", 8) == 0) ||
                (t.length == 4 && memcmp(t.start, "goto", 4) == 0)) {
                // These MUST be followed by semicolons (or value; for return)
                // Find end of statement
                int semi = find_next_semicolon(stream, i + 1);
                if (semi == -1) {
                    diag_add(diags, SEV_ERROR, t.line, t.column,
                        "Missing semicolon after '%.*s' statement", t.length, t.start);
                }
                i = semi > 0 ? semi : i + 1;
                continue;
            }
            // type declarations (struct, union, enum) - skip to ;
            if ((t.length == 6 && memcmp(t.start, "struct", 6) == 0) ||
                (t.length == 5 && memcmp(t.start, "union", 5) == 0) ||
                (t.length == 4 && memcmp(t.start, "enum", 4) == 0) ||
                (t.length == 8 && memcmp(t.start, "typedef", 8) == 0) ||
                (t.length == 6 && memcmp(t.start, "extern", 6) == 0) ||
                (t.length == 6 && memcmp(t.start, "static", 6) == 0) ||
                (t.length == 6 && memcmp(t.start, "inline", 6) == 0)) {
                int semi = find_next_semicolon(stream, i + 1);
                if (semi > 0) i = semi;
                continue;
            }
        }

        // Skip C++ keywords
        if (t.type == TOK_CPP_KEYWORD) {
            // class, namespace, template, etc. have blocks
            int semi = find_next_semicolon(stream, i + 1);
            if (semi > 0) i = semi;
            continue;
        }

        // Skip braces, parens, brackets (structural tokens)
        if (t.type == TOK_LBRACE || t.type == TOK_RBRACE ||
            t.type == TOK_LPAREN || t.type == TOK_RPAREN ||
            t.type == TOK_LBRACKET || t.type == TOK_RBRACKET) continue;

        // If we see a statement-ending token (identifier, literal, ), ]),
        // check if the next meaningful token is a semicolon or another statement start
        if (t.type == TOK_IDENT || t.type == TOK_INT_LIT || t.type == TOK_FLOAT_LIT ||
            t.type == TOK_CHAR_LIT || t.type == TOK_STRING_LIT || t.type == TOK_RPAREN ||
            t.type == TOK_RBRACKET || t.type == TOK_INC || t.type == TOK_DEC) {

            // Find next non-whitespace token
            int next = i + 1;
            while (next < stream->count) {
                TokenType nt = stream->tokens[next].type;
                if (nt != TOK_WHITESPACE && nt != TOK_NEWLINE &&
                    nt != TOK_COMMENT && nt != TOK_BLOCK_COMMENT)
                    break;
                next++;
            }

            if (next >= stream->count) continue;
            Token nxt = stream->tokens[next];

            // If next is a semicolon, fine
            if (nxt.type == TOK_SEMICOLON) { i = next; continue; }
            // If next is a closing bracket or brace, fine
            if (nxt.type == TOK_RBRACE || nxt.type == TOK_RPAREN || nxt.type == TOK_RBRACKET)
                continue;
            // If next is an operator that continues expression, skip to semicolon
            if (nxt.type == TOK_ASSIGN || nxt.type == TOK_EQ || nxt.type == TOK_NEQ ||
                nxt.type == TOK_LT || nxt.type == TOK_GT || nxt.type == TOK_LTE ||
                nxt.type == TOK_GTE || nxt.type == TOK_PLUS || nxt.type == TOK_MINUS ||
                nxt.type == TOK_STAR || nxt.type == TOK_SLASH || nxt.type == TOK_PERCENT ||
                nxt.type == TOK_AMP || nxt.type == TOK_PIPE || nxt.type == TOK_CARET ||
                nxt.type == TOK_LOG_AND || nxt.type == TOK_LOG_OR || nxt.type == TOK_LSHIFT ||
                nxt.type == TOK_RSHIFT || nxt.type == TOK_DOT || nxt.type == TOK_ARROW) {
                int semi = find_next_semicolon(stream, next + 1);
                if (semi > 0) { i = semi; continue; }
            }
            // If next is a comma (function args), skip to semicolon
            if (nxt.type == TOK_COMMA) {
                int semi = find_next_semicolon(stream, next + 1);
                if (semi > 0) { i = semi; continue; }
            }
            // If next is a keyword (control flow), continue
            if (nxt.type == TOK_KEYWORD || nxt.type == TOK_CPP_KEYWORD) continue;
            // If next is an identifier, it might be a label or next statement - check for colon
            if (nxt.type == TOK_IDENT) {
                // Could be a label: ident followed by colon
                int nn = next + 1;
                while (nn < stream->count && (stream->tokens[nn].type == TOK_WHITESPACE ||
                       stream->tokens[nn].type == TOK_NEWLINE)) nn++;
                if (nn < stream->count && stream->tokens[nn].type == TOK_COLON) continue; // label
                // Otherwise it's likely a missing semicolon
            }
            // If next is a type keyword, likely missing semicolon
            if (token_is_type(nxt)) {
                diag_add(diags, SEV_ERROR, t.line, t.column,
                    "Missing semicolon after expression");
            }
        }
    }
}

// ---- Analysis: Keywords ----

void analyze_keywords(TokenStream *stream, DiagnosticList *diags, int is_cpp) {
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type == TOK_KEYWORD) {
            diag_add(diags, SEV_INFO, t.line, t.column,
                "C keyword '%.*s'", t.length, t.start);
        }
        if (is_cpp && t.type == TOK_CPP_KEYWORD) {
            diag_add(diags, SEV_INFO, t.line, t.column,
                "C++ keyword '%.*s'", t.length, t.start);
        }
    }
}

// ---- Analysis: Functions ----

void analyze_functions(TokenStream *stream, DiagnosticList *diags) {
    int func_count = 0;
    int proto_count = 0;

    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];

        // Pattern: [type] identifier '(' ... ')' ';' or '{'
        // We look for: IDENT '('  where IDENT follows a type keyword or another IDENT
        if (t.type != TOK_IDENT) continue;

        // Check for opening paren after this identifier (possibly with *)
        int j = i + 1;
        while (j < stream->count) {
            Token jt = stream->tokens[j];
            if (jt.type == TOK_STAR || jt.type == TOK_WHITESPACE || jt.type == TOK_NEWLINE) {
                j++;
                continue;
            }
            break;
        }

        if (j >= stream->count || stream->tokens[j].type != TOK_LPAREN) continue;

        // Check if there's a type before the identifier
        int k = i - 1;
        while (k >= 0) {
            Token kt = stream->tokens[k];
            if (kt.type == TOK_WHITESPACE || kt.type == TOK_NEWLINE ||
                kt.type == TOK_COMMENT || kt.type == TOK_BLOCK_COMMENT) {
                k--;
                continue;
            }
            break;
        }

        if (k < 0) continue;
        Token prev = stream->tokens[k];

        bool has_type = token_is_type(prev) ||
            (prev.type == TOK_KEYWORD && (
                (prev.length == 6 && memcmp(prev.start, "struct", 6) == 0) ||
                (prev.length == 5 && memcmp(prev.start, "union", 5) == 0) ||
                (prev.length == 4 && memcmp(prev.start, "enum", 4) == 0) ||
                (prev.length == 8 && memcmp(prev.start, "unsigned", 8) == 0) ||
                (prev.length == 6 && memcmp(prev.start, "signed", 6) == 0) ||
                (prev.length == 4 && memcmp(prev.start, "long", 4) == 0) ||
                (prev.length == 5 && memcmp(prev.start, "short", 5) == 0) ||
                (prev.length == 4 && memcmp(prev.start, "void", 4) == 0) ||
                (prev.length == 4 && memcmp(prev.start, "char", 4) == 0) ||
                (prev.length == 3 && memcmp(prev.start, "int", 3) == 0) ||
                (prev.length == 5 && memcmp(prev.start, "float", 5) == 0) ||
                (prev.length == 6 && memcmp(prev.start, "double", 6) == 0) ||
                (prev.length == 4 && memcmp(prev.start, "bool", 4) == 0) ||
                (prev.length == 6 && memcmp(prev.start, "static", 6) == 0) ||
                (prev.length == 6 && memcmp(prev.start, "extern", 6) == 0) ||
                (prev.length == 6 && memcmp(prev.start, "inline", 6) == 0) ||
                (prev.length == 8 && memcmp(prev.start, "unsigned", 8) == 0)));

        if (!has_type) continue;

        // Skip function-like macros and declarations
        // Skip if name is a keyword
        if (t.type == TOK_KEYWORD) continue;

        // Find the closing paren
        int close_paren = find_matching_paren(stream, j);
        if (close_paren < 0) continue;

        // Check if it's a prototype (ends with ;) or a definition (has {)
        int after_paren = close_paren + 1;
        while (after_paren < stream->count) {
            Token at = stream->tokens[after_paren];
            if (at.type != TOK_WHITESPACE && at.type != TOK_NEWLINE &&
                at.type != TOK_COMMENT && at.type != TOK_BLOCK_COMMENT)
                break;
            after_paren++;
        }

        if (after_paren >= stream->count) continue;
        Token after = stream->tokens[after_paren];

        if (after.type == TOK_SEMICOLON) {
            proto_count++;
            i = after_paren;
        } else if (after.type == TOK_LBRACE || after.type == TOK_COMMA) {
            // Could be function definition (with {) or declaration with , (K&R style)
            if (after.type == TOK_LBRACE) {
                func_count++;
                int close_brace = find_matching_brace(stream, after_paren);
                if (close_brace > 0) i = close_brace;
            }
        }
    }

    diag_add(diags, SEV_INFO, 0, 0, "Functions: %d, Prototypes: %d", func_count, proto_count);
}

// ---- Analysis: Variables ----

void analyze_variables(TokenStream *stream, DiagnosticList *diags) {
    int count = 0;

    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];

        if (!token_is_type(t)) continue;

        // Skip if preceded by another type (e.g., "unsigned long int")
        // or by struct/union/enum keyword
        if (i > 0) {
            int prev = i - 1;
            while (prev >= 0 && (stream->tokens[prev].type == TOK_WHITESPACE ||
                   stream->tokens[prev].type == TOK_NEWLINE)) prev--;
            if (prev >= 0 && (token_is_type(stream->tokens[prev]) ||
                stream->tokens[prev].type == TOK_IDENT)) continue;
        }

        // Find the next identifier (the variable name)
        int j = i + 1;
        while (j < stream->count && (stream->tokens[j].type == TOK_WHITESPACE ||
               stream->tokens[j].type == TOK_NEWLINE ||
               stream->tokens[j].type == TOK_STAR)) j++;

        if (j >= stream->count || stream->tokens[j].type != TOK_IDENT) continue;

        // Skip if this is a function name followed by (
        int k = j + 1;
        while (k < stream->count && (stream->tokens[k].type == TOK_WHITESPACE ||
               stream->tokens[k].type == TOK_NEWLINE)) k++;
        if (k < stream->count && stream->tokens[k].type == TOK_LPAREN) continue;

        count++;

        // Check for common issues
        Token name = stream->tokens[j];

        // Check for single-letter variable names (style)
        if (name.length == 1 && *name.start != '_' &&
            *name.start != 'i' && *name.start != 'j' && *name.start != 'k' &&
            *name.start != 'n' && *name.start != 'x' && *name.start != 'y' &&
            *name.start != 'c') {
            diag_add(diags, SEV_STYLE, name.line, name.column,
                "Very short variable name '%.*s'", name.length, name.start);
        }

        // Check for snake_case in declarations (info, not warning)
        // Just report the declaration
        diag_add(diags, SEV_INFO, name.line, name.column,
            "Variable '%.*s' declared", name.length, name.start);
    }

    diag_add(diags, SEV_INFO, 0, 0, "Total variable declarations: %d", count);
}

// ---- Analysis: Keyword usage (loops, goto, etc.) ----

void analyze_keyword_usage(TokenStream *stream, DiagnosticList *diags) {
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type != TOK_KEYWORD) continue;

        if ((t.length == 3 && memcmp(t.start, "for", 3) == 0) ||
            (t.length == 5 && memcmp(t.start, "while", 5) == 0) ||
            (t.length == 2 && memcmp(t.start, "do", 2) == 0)) {
            diag_add(diags, SEV_INFO, t.line, t.column,
                "Loop '%.*s'", t.length, t.start);
        }
        if (t.length == 4 && memcmp(t.start, "goto", 4) == 0) {
            diag_add(diags, SEV_WARNING, t.line, t.column,
                "Use of 'goto' - consider refactoring");
        }
        if (t.length == 6 && memcmp(t.start, "switch", 6) == 0) {
            diag_add(diags, SEV_INFO, t.line, t.column, "Switch statement");
        }
    }
}

// ---- Analysis: Memory operations ----

void analyze_memory_ops(TokenStream *stream, DiagnosticList *diags) {
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type != TOK_IDENT) continue;

        bool is_alloc = (t.length == 6 && memcmp(t.start, "malloc", 6) == 0) ||
                        (t.length == 6 && memcmp(t.start, "calloc", 6) == 0) ||
                        (t.length == 6 && memcmp(t.start, "realloc", 6) == 0);
        bool is_free = (t.length == 4 && memcmp(t.start, "free", 4) == 0);
        bool is_cpp_alloc = (t.length == 3 && memcmp(t.start, "new", 3) == 0);
        bool is_cpp_free = (t.length == 6 && memcmp(t.start, "delete", 6) == 0);

        if (is_alloc) {
            diag_add(diags, SEV_INFO, t.line, t.column,
                "Memory allocation '%.*s'", t.length, t.start);

            // Check if return value is checked for NULL
            // Look for: if (ptr == NULL) or if (ptr != NULL) or if (ptr)
            // Simple heuristic: find the semicolon and look backwards for NULL check
            int semi = find_next_semicolon(stream, i);
            if (semi > 0) {
                bool checked = false;
                for (int j = i + 1; j < semi; j++) {
                    if (stream->tokens[j].type == TOK_EQ ||
                        stream->tokens[j].type == TOK_NEQ ||
                        stream->tokens[j].type == TOK_LOG_NOT) {
                        // Look for NULL nearby
                        for (int k = j - 2; k <= j + 2 && k < semi; k++) {
                            if (k >= 0 && stream->tokens[k].type == TOK_IDENT &&
                                stream->tokens[k].length == 4 &&
                                memcmp(stream->tokens[k].start, "NULL", 4) == 0) {
                                checked = true;
                                break;
                            }
                        }
                    }
                    if (checked) break;
                }
                if (!checked) {
                    diag_add(diags, SEV_WARNING, t.line, t.column,
                        "Return value of '%.*s' not checked for NULL",
                        t.length, t.start);
                }
            }
        }

        if (is_free) {
            diag_add(diags, SEV_INFO, t.line, t.column, "Memory deallocation 'free'");
        }
        if (is_cpp_alloc) {
            diag_add(diags, SEV_INFO, t.line, t.column, "C++ allocation 'new'");
        }
        if (is_cpp_free) {
            diag_add(diags, SEV_INFO, t.line, t.column, "C++ deallocation 'delete'");
        }
    }
}

// ---- Analysis: I/O operations ----

void analyze_io_ops(TokenStream *stream, DiagnosticList *diags) {
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type != TOK_IDENT) continue;

        bool is_print = (t.length == 6 && memcmp(t.start, "printf", 6) == 0) ||
                        (t.length == 5 && memcmp(t.start, "fprintf", 5) == 0) ||
                        (t.length == 7 && memcmp(t.start, "sprintf", 7) == 0) ||
                        (t.length == 7 && memcmp(t.start, "snprintf", 7) == 0);
        bool is_scan = (t.length == 6 && memcmp(t.start, "scanf", 6) == 0) ||
                       (t.length == 7 && memcmp(t.start, "fscanf", 7) == 0) ||
                       (t.length == 6 && memcmp(t.start, "sscanf", 6) == 0);

        if (is_print) {
            diag_add(diags, SEV_INFO, t.line, t.column,
                "Output function '%.*s'", t.length, t.start);
        }
        if (is_scan) {
            diag_add(diags, SEV_INFO, t.line, t.column,
                "Input function '%.*s'", t.length, t.start);
        }
    }
}

// ---- Analysis: File operations ----

void analyze_file_ops(TokenStream *stream, DiagnosticList *diags) {
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type != TOK_IDENT) continue;

        bool is_file = (t.length == 5 && memcmp(t.start, "fopen", 5) == 0) ||
                       (t.length == 6 && memcmp(t.start, "fclose", 6) == 0) ||
                       (t.length == 5 && memcmp(t.start, "fread", 5) == 0) ||
                       (t.length == 6 && memcmp(t.start, "fwrite", 6) == 0) ||
                       (t.length == 4 && memcmp(t.start, "feof", 4) == 0) ||
                       (t.length == 5 && memcmp(t.start, "fseek", 5) == 0) ||
                       (t.length == 5 && memcmp(t.start, "ftell", 5) == 0) ||
                       (t.length == 4 && memcmp(t.start, "rewind", 4) == 0);

        if (is_file) {
            diag_add(diags, SEV_INFO, t.line, t.column,
                "File operation '%.*s'", t.length, t.start);

            // Check if fopen return is checked for NULL
            if (t.length == 5 && memcmp(t.start, "fopen", 5) == 0) {
                int semi = find_next_semicolon(stream, i);
                if (semi > 0) {
                    bool checked = false;
                    for (int j = i + 1; j < semi; j++) {
                        if (stream->tokens[j].type == TOK_EQ ||
                            stream->tokens[j].type == TOK_NEQ ||
                            stream->tokens[j].type == TOK_LOG_NOT) {
                            checked = true;
                            break;
                        }
                    }
                    if (!checked) {
                        diag_add(diags, SEV_WARNING, t.line, t.column,
                            "Return value of 'fopen' not checked for NULL");
                    }
                }
            }
        }
    }
}

// ---- Analysis: C++ specific constructs ----

void analyze_cpp_constructs(TokenStream *stream, DiagnosticList *diags) {
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type != TOK_CPP_KEYWORD) continue;

        if ((t.length == 5 && memcmp(t.start, "class", 5) == 0) ||
            (t.length == 9 && memcmp(t.start, "namespace", 9) == 0) ||
            (t.length == 8 && memcmp(t.start, "template", 8) == 0) ||
            (t.length == 7 && memcmp(t.start, "virtual", 7) == 0) ||
            (t.length == 9 && memcmp(t.start, "constexpr", 9) == 0) ||
            (t.length == 8 && memcmp(t.start, "override", 8) == 0) ||
            (t.length == 8 && memcmp(t.start, "noexcept", 8) == 0) ||
            (t.length == 7 && memcmp(t.start, "nullptr", 7) == 0) ||
            (t.length == 8 && memcmp(t.start, "explicit", 8) == 0) ||
            (t.length == 8 && memcmp(t.start, "mutable", 8) == 0) ||
            (t.length == 6 && memcmp(t.start, "friend", 6) == 0) ||
            (t.length == 7 && memcmp(t.start, "public", 7) == 0) ||
            (t.length == 8 && memcmp(t.start, "private", 8) == 0) ||
            (t.length == 10 && memcmp(t.start, "protected", 10) == 0)) {
            diag_add(diags, SEV_INFO, t.line, t.column,
                "C++ construct '%.*s'", t.length, t.start);
        }
    }
}

// ---- Cyclomatic complexity ----

int calculate_complexity(TokenStream *stream) {
    int complexity = 1;
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type == TOK_KEYWORD) {
            if ((t.length == 2 && memcmp(t.start, "if", 2) == 0) ||
                (t.length == 3 && memcmp(t.start, "for", 3) == 0) ||
                (t.length == 5 && memcmp(t.start, "while", 5) == 0) ||
                (t.length == 4 && memcmp(t.start, "case", 4) == 0) ||
                (t.length == 7 && memcmp(t.start, "default", 7) == 0 && i > 0 &&
                 stream->tokens[i-1].type == TOK_COLON) ||
                (t.length == 6 && memcmp(t.start, "switch", 6) == 0)) {
                complexity++;
            }
        }
        if (t.type == TOK_CPP_KEYWORD &&
            ((t.length == 5 && memcmp(t.start, "catch", 5) == 0))) {
            complexity++;
        }
        if (t.type == TOK_LOG_AND || t.type == TOK_LOG_OR) {
            complexity++;
        }
        // ? : ternary
        if (t.type == TOK_QMARK) complexity++;
    }
    return complexity;
}

// ---- Analysis: Memory leak detection ----

void analyze_memory_leaks(TokenStream *stream, DiagnosticList *diags) {
    int alloc_count = 0;
    int free_count = 0;
    int alloc_lines[1024];

    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type != TOK_IDENT) continue;

        bool is_alloc = (t.length == 6 && memcmp(t.start, "malloc", 6) == 0) ||
                        (t.length == 6 && memcmp(t.start, "calloc", 6) == 0) ||
                        (t.length == 6 && memcmp(t.start, "realloc", 6) == 0);
        bool is_free = (t.length == 4 && memcmp(t.start, "free", 4) == 0);

        if (is_alloc && alloc_count < 1024) {
            alloc_lines[alloc_count++] = t.line;
        }
        if (is_free) free_count++;
    }

    if (alloc_count > free_count) {
        for (int i = free_count; i < alloc_count && i < 1024; i++) {
            diag_add(diags, SEV_WARNING, alloc_lines[i], 0,
                "Potential memory leak: allocation without matching free");
        }
    }
}

// ---- Analysis: goto ----

void analyze_goto(TokenStream *stream, DiagnosticList *diags) {
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type == TOK_KEYWORD && t.length == 4 && memcmp(t.start, "goto", 4) == 0) {
            diag_add(diags, SEV_WARNING, t.line, t.column,
                "Use of 'goto' - reduces code clarity");
        }
    }
}

// ---- Analysis: Unused / uninitialized / shadowed variables ----

static bool next_token_is(TokenStream *s, int i, TokenType type) {
    int j = i + 1;
    while (j < s->count && (s->tokens[j].type == TOK_WHITESPACE || s->tokens[j].type == TOK_NEWLINE)) j++;
    return j < s->count && s->tokens[j].type == type;
}

// Shared helper: is token at index i a variable declaration?
static bool decl_info(TokenStream *s, int i, char *name_out, int *name_len,
                      bool *is_pointer, bool *initialized) {
    Token t = s->tokens[i];
    if (t.type != TOK_IDENT) return false;

    int j = i - 1;
    while (j >= 0 && (s->tokens[j].type == TOK_WHITESPACE ||
           s->tokens[j].type == TOK_NEWLINE || s->tokens[j].type == TOK_STAR)) j--;
    if (j < 0 || !token_is_type(s->tokens[j])) return false;

    if (next_token_is(s, i, TOK_LPAREN)) return false;

    bool ptr_seen = false;
    for (int k = j + 1; k < i; k++) {
        if (s->tokens[k].type == TOK_STAR) ptr_seen = true;
    }

    bool init = next_token_is(s, i, TOK_ASSIGN) ||
                next_token_is(s, i, TOK_LPAREN) ||
                next_token_is(s, i, TOK_LBRACKET);

    int nn = t.length < 63 ? t.length : 63;
    memcpy(name_out, t.start, nn);
    name_out[nn] = '\0';
    if (name_len) *name_len = nn;
    if (is_pointer) *is_pointer = ptr_seen;
    if (initialized) *initialized = init;
    return true;
}

void analyze_unused_variables(TokenStream *stream, DiagnosticList *diags) {
    typedef struct { char name[64]; int line, col; int ref; } UEntry;
    UEntry *vars = NULL; int vcount = 0, vcap = 0;

    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type != TOK_IDENT) continue;

        char name[64]; bool isinit, isptr; int nl;
        int tn = t.length < 63 ? t.length : 63;
        memcpy(name, t.start, tn); name[tn] = '\0';

        if (decl_info(stream, i, name, &nl, &isptr, &isinit)) {
            if (vcount >= vcap) {
                vcap = vcap ? vcap * 2 : 32;
                vars = realloc(vars, vcap * sizeof(UEntry));
            }
            memcpy(vars[vcount].name, name, nl + 1);
            vars[vcount].line = t.line;
            vars[vcount].col = t.column;
            vars[vcount].ref = 0;
            vcount++;
        } else {
            for (int k = 0; k < vcount; k++) {
                if (strcmp(vars[k].name, name) == 0) { vars[k].ref++; break; }
            }
        }
    }

    for (int k = 0; k < vcount; k++) {
        if (vars[k].ref == 0) {
            diag_add(diags, SEV_WARNING, vars[k].line, vars[k].col,
                "Variable '%s' declared but never used", vars[k].name);
        }
    }

    free(vars);
}

void analyze_uninitialized_variables(TokenStream *stream, DiagnosticList *diags) {
    typedef struct { char name[64]; int line, col; bool init; } VEntry;
    VEntry *vars = NULL; int vcount = 0, vcap = 0;

    // Precompute which token indices lie inside a function's parameter list,
    // so that parameter declarations are treated as initialized (by the caller).
    int *in_params = calloc(stream->count, sizeof(int));
    for (int k = 0; k < stream->count; k++) {
        if (stream->tokens[k].type != TOK_LPAREN) continue;
        // preceding meaningful token must be an identifier (function name)
        int prev = k - 1;
        while (prev >= 0 && (stream->tokens[prev].type == TOK_WHITESPACE ||
               stream->tokens[prev].type == TOK_NEWLINE)) prev--;
        if (prev < 0 || stream->tokens[prev].type != TOK_IDENT) continue;
        // matching ')' followed by '{' marks a function definition
        int m = find_matching_paren(stream, k);
        if (m < 0 || m >= stream->count) continue;
        int body = m + 1;
        while (body < stream->count && (stream->tokens[body].type == TOK_WHITESPACE ||
               stream->tokens[body].type == TOK_NEWLINE)) body++;
        if (body >= stream->count || stream->tokens[body].type != TOK_LBRACE) continue;
        for (int x = k + 1; x < m; x++) in_params[x] = 1;
        k = m;
    }

    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type != TOK_IDENT) continue;

        char name[64]; bool isinit, isptr; int nl;
        int tn = t.length < 63 ? t.length : 63;
        memcpy(name, t.start, tn); name[tn] = '\0';

        if (decl_info(stream, i, name, &nl, &isptr, &isinit)) {
            if (vcount >= vcap) {
                vcap = vcap ? vcap * 2 : 32;
                vars = realloc(vars, vcap * sizeof(VEntry));
            }
            memcpy(vars[vcount].name, name, nl + 1);
            vars[vcount].line = t.line;
            vars[vcount].col = t.column;
            vars[vcount].init = isinit || in_params[i];
            vcount++;
        } else {
            for (int k = 0; k < vcount; k++) {
                if (strcmp(vars[k].name, name) == 0) {
                    if (!vars[k].init) {
                        bool is_lhs = next_token_is(stream, i, TOK_ASSIGN);
                        if (!is_lhs) {
                            diag_add(diags, SEV_WARNING, t.line, t.column,
                                "Variable '%s' may be used before initialization", name);
                        }
                    }
                    break;
                }
            }
        }
    }

    free(in_params);
    free(vars);
}

void analyze_shadowed_variables(TokenStream *stream, DiagnosticList *diags) {
    #define MAX_LEVELS 32
    typedef struct { char *names[256]; int count; } Lev;
    Lev levels[MAX_LEVELS]; memset(levels, 0, sizeof(levels));
    int depth = 0;

    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type == TOK_LBRACE) { depth++; if (depth >= MAX_LEVELS) depth = MAX_LEVELS-1; continue; }
        if (t.type == TOK_RBRACE) { if (depth > 0) depth--; continue; }
        if (t.type != TOK_IDENT) continue;

        char name[64]; bool isinit, isptr; int nl;
        if (!decl_info(stream, i, name, &nl, &isptr, &isinit)) continue;

        for (int lvl = depth - 1; lvl >= 0; lvl--) {
            for (int v = 0; v < levels[lvl].count; v++) {
                if (strcmp(levels[lvl].names[v], name) == 0) {
                    diag_add(diags, SEV_WARNING, t.line, t.column,
                        "Variable '%s' shadows an outer variable", name);
                    goto recorded;
                }
            }
        }
        if (levels[depth].count < 256) {
            char *c = malloc(nl + 1); memcpy(c, name, nl + 1);
            levels[depth].names[levels[depth].count++] = c;
        }
        recorded: ;
    }

    for (int l = 0; l < MAX_LEVELS; l++)
        for (int v = 0; v < levels[l].count; v++) free(levels[l].names[v]);
}

// ---- Analysis: Null pointer dereference ----

void analyze_null_pointer_deref(TokenStream *stream, DiagnosticList *diags) {
    typedef struct { char name[64]; bool init, isptr, null_assigned; int line, col; } NEntry;
    NEntry *vars = NULL; int vcount = 0, vcap = 0;

    // Track local pointer variables, whether they were initialized, and whether
    // they were assigned NULL/0 at declaration (a null-dereference candidate).
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type != TOK_IDENT) continue;

        char name[64]; bool isinit, isptr; int nl;
        int tn = t.length < 63 ? t.length : 63;
        memcpy(name, t.start, tn); name[tn] = '\0';

        if (decl_info(stream, i, name, &nl, &isptr, &isinit)) {
            if (vcount >= vcap) {
                vcap = vcap ? vcap * 2 : 32;
                vars = realloc(vars, vcap * sizeof(NEntry));
            }
            memcpy(vars[vcount].name, name, nl + 1);
            vars[vcount].init = isinit;
            vars[vcount].isptr = isptr;
            vars[vcount].line = t.line;
            vars[vcount].col = t.column;
            // pointer not initialized at declaration, or initialized with NULL/0,
            // is a candidate for null dereference
            vars[vcount].null_assigned = 0;
            if (isptr) {
                // scan for '=' NULL or '=' 0 after the declaration name
                int e = i + 1;
                while (e < stream->count && (stream->tokens[e].type == TOK_WHITESPACE ||
                       stream->tokens[e].type == TOK_NEWLINE)) e++;
                if (e < stream->count && stream->tokens[e].type == TOK_ASSIGN) {
                    e++;
                    while (e < stream->count && (stream->tokens[e].type == TOK_WHITESPACE ||
                           stream->tokens[e].type == TOK_NEWLINE)) e++;
                    if (e < stream->count) {
                        Token v = stream->tokens[e];
                        bool isnull = (v.type == TOK_IDENT && v.length == 4 &&
                                       memcmp(v.start, "NULL", 4) == 0) ||
                                      (v.type == TOK_INT_LIT && v.length == 1 &&
                                       v.start[0] == '0');
                        if (isnull) vars[vcount].null_assigned = 1;
                    }
                }
            }
            vcount++;
            continue;
        }

        // Reference: if it's a pointer dereference, check declared/assigned null state.
        int p = i - 1;
        while (p >= 0 && (stream->tokens[p].type == TOK_WHITESPACE ||
               stream->tokens[p].type == TOK_NEWLINE)) p--;
        bool deref = (p >= 0 && stream->tokens[p].type == TOK_STAR);

        for (int k = 0; k < vcount; k++) {
            if (strcmp(vars[k].name, name) == 0 && vars[k].isptr) {
                if (deref && (!vars[k].init || vars[k].null_assigned)) {
                    diag_add(diags, SEV_ERROR, t.line, t.column,
                        "Possible null pointer dereference: '*%s'", name);
                }
                break;
            }
        }
    }

    free(vars);
}

// ---- Analysis: Buffer overflow heuristic ----

void analyze_buffer_overflow(TokenStream *stream, DiagnosticList *diags) {
    // Look for array declarations: type name [N]  then later  name[const>=N]
    #define MAX_ARRAYS 128
    struct { char name[64]; int size; int line; } arrays[MAX_ARRAYS];
    int acount = 0;

    for (int i = 0; i < stream->count && acount < MAX_ARRAYS; i++) {
        Token t = stream->tokens[i];
        if (t.type != TOK_IDENT) continue;

        char name[64]; bool isinit, isptr; int nl;
        if (!decl_info(stream, i, name, &nl, &isptr, &isinit)) continue;

        // name then [ int-literal ]  -> array with fixed size
        int idx = i + 1;
        while (idx < stream->count && (stream->tokens[idx].type==TOK_WHITESPACE||stream->tokens[idx].type==TOK_NEWLINE)) idx++;
        if (idx >= stream->count || stream->tokens[idx].type!=TOK_LBRACKET) continue;
        idx++;
        while (idx < stream->count && (stream->tokens[idx].type==TOK_WHITESPACE||stream->tokens[idx].type==TOK_NEWLINE)) idx++;
        if (idx >= stream->count || stream->tokens[idx].type!=TOK_INT_LIT) continue;
        int size = atoi(stream->tokens[idx].start);
        memcpy(arrays[acount].name, name, nl + 1);
        arrays[acount].size = size;
        arrays[acount].line = t.line;
        acount++;
    }

    // Now scan for accesses name[constant] >= size
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type != TOK_IDENT) continue;
        // Skip if this ident starts a declaration (its own [N]) - not an access
        {
            char ref[64]; bool ri, rp; int rl;
            if (decl_info(stream, i, ref, &rl, &rp, &ri)) continue;
        }
        for (int a = 0; a < acount; a++) {
            if (t.length == (int)strlen(arrays[a].name) &&
                memcmp(t.start, arrays[a].name, t.length) == 0) {
                // find following '[' and constant
                int idx = i + 1;
                while (idx < stream->count && (stream->tokens[idx].type==TOK_WHITESPACE||
                       stream->tokens[idx].type==TOK_NEWLINE)) idx++;
                if (idx < stream->count && stream->tokens[idx].type==TOK_LBRACKET) {
                    idx++;
                    while (idx < stream->count && (stream->tokens[idx].type==TOK_WHITESPACE||
                           stream->tokens[idx].type==TOK_NEWLINE)) idx++;
                    if (idx < stream->count && stream->tokens[idx].type==TOK_INT_LIT) {
                        int value = atoi(stream->tokens[idx].start);
                        if (value >= arrays[a].size) {
                            diag_add(diags, SEV_WARNING, t.line, t.column,
                                "Possible out-of-bounds array access: '%s[%d]' (size %d)",
                                arrays[a].name, value, arrays[a].size);
                        }
                    }
                }
                break;
            }
        }
    }
}

// ---- Analysis: Format string ----

void analyze_format_strings(TokenStream *stream, DiagnosticList *diags) {
    // For printf("...", args...): count %-specifiers vs number of arguments.
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type == TOK_IDENT &&
            ((t.length == 6 && memcmp(t.start, "printf", 6) == 0) ||
             (t.length == 7 && memcmp(t.start, "snprintf", 7) == 0) ||
             (t.length == 5 && memcmp(t.start, "scanf", 5) == 0))) {
            // find next string literal
            int j = i + 1;
            while (j < stream->count && (stream->tokens[j].type==TOK_WHITESPACE||
                   stream->tokens[j].type==TOK_NEWLINE)) j++;
            if (j < stream->count && stream->tokens[j].type==TOK_LPAREN) {
                // find opening string literal after '('
                int k = j + 1;
                while (k < stream->count && (stream->tokens[k].type==TOK_WHITESPACE||
                       stream->tokens[k].type==TOK_NEWLINE)) k++;
                if (k < stream->count && stream->tokens[k].type==TOK_STRING_LIT) {
                    const char *fmt = stream->tokens[k].start;
                    int fmt_len = stream->tokens[k].length;
                    // count % specifiers (skip %%)
                    int spec = 0;
                    for (int c = 1; c < fmt_len-1; c++) {
                        if (fmt[c]=='%') {
                            if (fmt[c+1]=='%') { c++; continue; }
                            spec++;
                        }
                    }
                    // count commas after the format up to matching close paren
                    int close = find_matching_paren(stream, j);
                    int commas = 0;
                    for (int c = k; c < close; c++)
                        if (stream->tokens[c].type==TOK_COMMA) commas++;
                    if (spec != commas && !(fmt[0]=='%' && fmt[1]=='s' && commas==0 && spec==0)) {
                        // Only warn if reasonable: e.g. literal error or missing arg
                        if (spec > commas && !(spec==0 && commas==0))
                            diag_add(diags, SEV_WARNING, t.line, t.column,
                                "Format string expects %d argument(s) but %d passed", spec, commas);
                        else if (commas > spec)
                            diag_add(diags, SEV_WARNING, t.line, t.column,
                                "More arguments (%d) than format specifiers (%d)", commas, spec);
                    }
                }
            }
        }
    }
}

// ---- Analysis: Switch fallthrough ----

void analyze_switch_fallthrough(TokenStream *stream, DiagnosticList *diags) {
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type == TOK_KEYWORD && t.length == 6 && memcmp(t.start, "switch", 6) == 0) {
            // find the switch body brace
            int j = i + 1;
            while (j < stream->count && (stream->tokens[j].type==TOK_WHITESPACE||
                   stream->tokens[j].type==TOK_NEWLINE)) j++;
            if (j < stream->count && stream->tokens[j].type==TOK_LPAREN) {
                int close = find_matching_paren(stream, j);
                int body = close + 1;
                while (body < stream->count && (stream->tokens[body].type==TOK_WHITESPACE||
                       stream->tokens[body].type==TOK_NEWLINE)) body++;
                if (body < stream->count && stream->tokens[body].type==TOK_LBRACE) {
                    int match = find_matching_brace(stream, body);
                    // scan between body..match for 'case X: ...' that lacks 'break;'
                    int case_before = -1;
                    for (int k = body+1; k < match; k++) {
                        Token c = stream->tokens[k];
                        if (c.type == TOK_KEYWORD &&
                            ((c.length==4 && memcmp(c.start,"case",4)==0) ||
                             (c.length==7 && memcmp(c.start,"default",7)==0))) {
                            // record that a new case started
                            case_before = k;
                            // check the range since last case for missing break
                            // we'll do reverse scan separately; simplified here:
                        }
                        if (case_before >= 0 && c.type == TOK_KEYWORD &&
                            c.length==5 && memcmp(c.start,"break",5)==0) {
                            case_before = -1; // cleared by break
                        }
                        // heuristic: if we reach another case while previous had no break
                        if (case_before >= 0 && c.type != TOK_KEYWORD) { /* ignore */ }
                    }
                    // Simpler: for each case block, ensure a break (or return/throw) before next case
                    for (int k = body+1; k < match; k++) {
                        Token c = stream->tokens[k];
                        if (c.type == TOK_KEYWORD &&
                            ((c.length==4 && memcmp(c.start,"case",4)==0) ||
                             (c.length==7 && memcmp(c.start,"default",7)==0))) {
                            // scan forward until next case/default or closing switch brace
                            bool has_break = false;
                            for (int m = k+1; m < match; m++) {
                                Token cn = stream->tokens[m];
                                if (cn.type == TOK_KEYWORD &&
                                    ((cn.length==4 && memcmp(cn.start,"case",4)==0) ||
                                     (cn.length==7 && memcmp(cn.start,"default",7)==0) ||
                                     (cn.length==5 && memcmp(cn.start,"break",5)==0) ||
                                     (cn.length==4 && memcmp(cn.start,"goto",4)==0) ||
                                     (cn.length==6 && memcmp(cn.start,"return",6)==0))) {
                                    if (cn.length==5 && memcmp(cn.start,"break",5)==0) {
                                        has_break = true;
                                        k = m; break;
                                    }
                                    if (cn.length==6 && memcmp(cn.start,"return",6)==0) {
                                        has_break = true; k = m; break;
                                    }
                                    if (cn.length==4 && memcmp(cn.start,"goto",4)==0) {
                                        has_break = true; k = m; break;
                                    }
                                    // reached next case without break -> fallthrough
                                    if ((cn.length==4 && memcmp(cn.start,"case",4)==0) ||
                                        (cn.length==7 && memcmp(cn.start,"default",7)==0)) {
                                        if (!has_break) {
                                            diag_add(diags, SEV_WARNING, c.line, c.column,
                                                "Possible implicit fallthrough in switch case");
                                        }
                                        has_break = true; // avoid dup
                                        break;
                                    }
                                }
                            }
                            (void)has_break;
                        }
                    }
                    i = match;
                }
            }
        }
    }
}

// ---- Analysis: Dead code ----

void analyze_dead_code(TokenStream *stream, DiagnosticList *diags) {
    // Statements after return/break/continue/goto within the same block (up to next '}') are unreachable.
    int depth = 0;
    for (int i = 0; i < stream->count; i++) {
        Token t = stream->tokens[i];
        if (t.type == TOK_LBRACE) { depth++; continue; }
        if (t.type == TOK_RBRACE) { if(depth>0)depth--; continue; }
        if (t.type == TOK_KEYWORD &&
            ((t.length==6 && memcmp(t.start,"return",6)==0) ||
             (t.length==6 && memcmp(t.start,"break",6)==0) ||
             (t.length==8 && memcmp(t.start,"continue",8)==0) ||
             (t.length==4 && memcmp(t.start,"goto",4)==0))) {
            // find the terminating semicolon then next real token
            int semi = find_next_semicolon(stream, i+1);
            if (semi < 0) continue;
            int nxt = semi + 1;
            while (nxt < stream->count && (stream->tokens[nxt].type==TOK_WHITESPACE||
                   stream->tokens[nxt].type==TOK_NEWLINE||stream->tokens[nxt].type==TOK_COMMENT||
                   stream->tokens[nxt].type==TOK_BLOCK_COMMENT)) nxt++;
            if (nxt < stream->count) {
                Token nt = stream->tokens[nxt];
                if (nt.type == TOK_RBRACE) { /* end of block - legitimate */ }
                else if (nt.type == TOK_SEMICOLON) { /* do nothing */ }
                else {
                    diag_add(diags, SEV_WARNING, nt.line, nt.column,
                        "Unreachable code after '%.*s' statement", t.length, t.start);
                }
            }
        }
    }
}

// ---- Analysis: Nesting depth ----

void analyze_nesting_depth(TokenStream *stream, DiagnosticList *diags) {
    int depth = 0;
    int max = 0;
    for (int i = 0; i < stream->count; i++) {
        if (stream->tokens[i].type == TOK_LBRACE) { depth++; if(depth>max)max=depth; }
        else if (stream->tokens[i].type == TOK_RBRACE) { if(depth>0)depth--; }
    }
    if (max > 6) {
        diag_add(diags, SEV_STYLE, 0, 0,
            "Deep nesting detected (%d levels) - consider refactoring", max);
    }
    if (max > 0) {
        diag_add(diags, SEV_INFO, 0, 0, "Maximum nesting depth: %d", max);
    }
}

// ---- Output ----

void print_diagnostics(DiagnosticList *diags, FILE *output) {
    const char *sev_names[] = {"Error", "Warning", "Info", "Style"};

    int errors = 0, warnings = 0, infos = 0, styles = 0;

    for (int i = 0; i < diags->count; i++) {
        Diagnostic *d = &diags->items[i];
        if (d->line > 0) {
            fprintf(output, "%s at line %d, col %d: %s\n",
                    sev_names[d->severity], d->line, d->column, d->message);
        } else {
            fprintf(output, "%s: %s\n", sev_names[d->severity], d->message);
        }
        switch (d->severity) {
            case SEV_ERROR: errors++; break;
            case SEV_WARNING: warnings++; break;
            case SEV_INFO: infos++; break;
            case SEV_STYLE: styles++; break;
        }
    }

    fprintf(output, "\n--- Summary ---\n");
    fprintf(output, "Errors:   %d\n", errors);
    fprintf(output, "Warnings: %d\n", warnings);
    fprintf(output, "Info:     %d\n", infos);
    fprintf(output, "Style:    %d\n", styles);
    fprintf(output, "Total:    %d\n", diags->count);
}

void print_file_lines(const char *source, FILE *output) {
    int line_num = 1;
    fprintf(output, "Line %d: ", line_num);
    for (const char *p = source; *p; p++) {
        fputc(*p, output);
        if (*p == '\n') {
            line_num++;
            fprintf(output, "Line %d: ", line_num);
        }
    }
    fprintf(output, "\n");
}
