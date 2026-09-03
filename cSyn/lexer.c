#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ---- C keywords (87 total) ----
static const KeywordEntry c_keywords[] = {
    {"auto", TOK_KEYWORD}, {"break", TOK_KEYWORD}, {"case", TOK_KEYWORD},
    {"char", TOK_KEYWORD}, {"const", TOK_KEYWORD}, {"continue", TOK_KEYWORD},
    {"default", TOK_KEYWORD}, {"do", TOK_KEYWORD}, {"double", TOK_KEYWORD},
    {"else", TOK_KEYWORD}, {"enum", TOK_KEYWORD}, {"extern", TOK_KEYWORD},
    {"float", TOK_KEYWORD}, {"for", TOK_KEYWORD}, {"goto", TOK_KEYWORD},
    {"if", TOK_KEYWORD}, {"inline", TOK_KEYWORD}, {"int", TOK_KEYWORD},
    {"long", TOK_KEYWORD}, {"register", TOK_KEYWORD}, {"restrict", TOK_KEYWORD},
    {"return", TOK_KEYWORD}, {"short", TOK_KEYWORD}, {"signed", TOK_KEYWORD},
    {"sizeof", TOK_KEYWORD}, {"static", TOK_KEYWORD}, {"struct", TOK_KEYWORD},
    {"switch", TOK_KEYWORD}, {"typedef", TOK_KEYWORD}, {"union", TOK_KEYWORD},
    {"unsigned", TOK_KEYWORD}, {"void", TOK_KEYWORD}, {"volatile", TOK_KEYWORD},
    {"while", TOK_KEYWORD}, {"_Alignas", TOK_KEYWORD}, {"_Alignof", TOK_KEYWORD},
    {"_Atomic", TOK_KEYWORD}, {"_Bool", TOK_KEYWORD}, {"_Complex", TOK_KEYWORD},
    {"_Generic", TOK_KEYWORD}, {"_Imaginary", TOK_KEYWORD},
    {"_Noreturn", TOK_KEYWORD}, {"_Static_assert", TOK_KEYWORD},
    {"_Thread_local", TOK_KEYWORD},
    {NULL, TOK_ERROR}
};

// ---- C++ keywords ----
static const KeywordEntry cpp_keywords[] = {
    {"alignas", TOK_CPP_KEYWORD}, {"alignof", TOK_CPP_KEYWORD},
    {"and", TOK_CPP_KEYWORD}, {"and_eq", TOK_CPP_KEYWORD},
    {"asm", TOK_CPP_KEYWORD}, {"bitand", TOK_CPP_KEYWORD},
    {"bitor", TOK_CPP_KEYWORD}, {"bool", TOK_CPP_KEYWORD},
    {"catch", TOK_CPP_KEYWORD}, {"char8_t", TOK_CPP_KEYWORD},
    {"char16_t", TOK_CPP_KEYWORD}, {"char32_t", TOK_CPP_KEYWORD},
    {"class", TOK_CPP_KEYWORD}, {"compl", TOK_CPP_KEYWORD},
    {"concept", TOK_CPP_KEYWORD}, {"consteval", TOK_CPP_KEYWORD},
    {"constexpr", TOK_CPP_KEYWORD}, {"constinit", TOK_CPP_KEYWORD},
    {"const_cast", TOK_CPP_KEYWORD}, {"co_await", TOK_CPP_KEYWORD},
    {"co_return", TOK_CPP_KEYWORD}, {"co_yield", TOK_CPP_KEYWORD},
    {"decltype", TOK_CPP_KEYWORD}, {"delete", TOK_CPP_KEYWORD},
    {"dynamic_cast", TOK_CPP_KEYWORD}, {"explicit", TOK_CPP_KEYWORD},
    {"export", TOK_CPP_KEYWORD}, {"false", TOK_CPP_KEYWORD},
    {"friend", TOK_CPP_KEYWORD}, {"mutable", TOK_CPP_KEYWORD},
    {"namespace", TOK_CPP_KEYWORD}, {"new", TOK_CPP_KEYWORD},
    {"noexcept", TOK_CPP_KEYWORD}, {"not", TOK_CPP_KEYWORD},
    {"not_eq", TOK_CPP_KEYWORD}, {"nullptr", TOK_CPP_KEYWORD},
    {"operator", TOK_CPP_KEYWORD}, {"or", TOK_CPP_KEYWORD},
    {"or_eq", TOK_CPP_KEYWORD}, {"private", TOK_CPP_KEYWORD},
    {"protected", TOK_CPP_KEYWORD}, {"public", TOK_CPP_KEYWORD},
    {"reinterpret_cast", TOK_CPP_KEYWORD},
    {"requires", TOK_CPP_KEYWORD},
    {"static_assert", TOK_CPP_KEYWORD},
    {"static_cast", TOK_CPP_KEYWORD},
    {"template", TOK_CPP_KEYWORD}, {"this", TOK_CPP_KEYWORD},
    {"thread_local", TOK_CPP_KEYWORD}, {"throw", TOK_CPP_KEYWORD},
    {"true", TOK_CPP_KEYWORD}, {"try", TOK_CPP_KEYWORD},
    {"typeid", TOK_CPP_KEYWORD}, {"typename", TOK_CPP_KEYWORD},
    {"using", TOK_CPP_KEYWORD}, {"virtual", TOK_CPP_KEYWORD},
    {"wchar_t", TOK_CPP_KEYWORD},
    {"xor", TOK_CPP_KEYWORD}, {"xor_eq", TOK_CPP_KEYWORD},
    {NULL, TOK_ERROR}
};

// ---- Internal helpers ----

static char advance(Lexer *l) {
    char c = *l->current++;
    if (c == '\n') { l->line++; l->column = 1; }
    else { l->column++; }
    return c;
}

static char peek_char(Lexer *l) { return *l->current; }

static char peek_char_at(Lexer *l, int offset) { return l->current[offset]; }

static bool match(Lexer *l, char expected) {
    if (*l->current == expected) { advance(l); return true; }
    return false;
}

static bool is_at_end(Lexer *l) { return *l->current == '\0'; }

static Token make_token(Lexer *l, TokenType type, int start_line, int start_col) {
    Token t;
    t.type = type;
    t.start = l->start;
    t.length = (int)(l->current - l->start);
    t.line = start_line;
    t.column = start_col;
    return t;
}

static Token error_token(Lexer *l, int start_line, int start_col) {
    return make_token(l, TOK_ERROR, start_line, start_col);
}

// Skip block comment, returns true if we're still in a block comment at EOF
static bool skip_block_comment(Lexer *l) {
    while (!is_at_end(l)) {
        if (*l->current == '*' && peek_char_at(l, 1) == '/') {
            advance(l); // *
            advance(l); // /
            return false;
        }
        advance(l);
    }
    return true; // unterminated block comment
}

// Skip whitespace and single-line comments
static void skip_whitespace(Lexer *l) {
    for (;;) {
        if (is_at_end(l)) return;
        char c = peek_char(l);
        if (c == ' ' || c == '\t' || c == '\r') { advance(l); }
        else if (c == '\n') { advance(l); }
        else if (c == '/' && peek_char_at(l, 1) == '/') {
            while (!is_at_end(l) && peek_char(l) != '\n') advance(l);
        }
        else if (c == '/' && peek_char_at(l, 1) == '*') {
            advance(l); advance(l);
            l->in_block_comment = skip_block_comment(l);
        }
        else break;
    }
}

// ---- Tokenizers for specific kinds ----

static Token tokenize_string(Lexer *l) {
    int sl = l->line, sc = l->column;
    advance(l); // opening "
    while (!is_at_end(l) && peek_char(l) != '"') {
        if (peek_char(l) == '\\') advance(l);
        if (!is_at_end(l)) advance(l);
    }
    if (is_at_end(l)) return error_token(l, sl, sc);
    advance(l); // closing "
    return make_token(l, TOK_STRING_LIT, sl, sc);
}

static Token tokenize_char(Lexer *l) {
    int sl = l->line, sc = l->column;
    advance(l); // opening '
    while (!is_at_end(l) && peek_char(l) != '\'') {
        if (peek_char(l) == '\\') advance(l);
        if (!is_at_end(l)) advance(l);
    }
    if (is_at_end(l)) return error_token(l, sl, sc);
    advance(l); // closing '
    return make_token(l, TOK_CHAR_LIT, sl, sc);
}

static Token tokenize_number(Lexer *l) {
    int sl = l->line, sc = l->column;
    bool is_float = false;

    if (peek_char(l) == '0' && (peek_char_at(l, 1) == 'x' || peek_char_at(l, 1) == 'X')) {
        advance(l); advance(l);
        while (!is_at_end(l) && isxdigit((unsigned char)peek_char(l))) advance(l);
    } else {
        while (!is_at_end(l) && isdigit((unsigned char)peek_char(l))) advance(l);
        if (!is_at_end(l) && peek_char(l) == '.') {
            is_float = true;
            advance(l);
            while (!is_at_end(l) && isdigit((unsigned char)peek_char(l))) advance(l);
        }
        if (!is_at_end(l) && (peek_char(l) == 'e' || peek_char(l) == 'E')) {
            is_float = true;
            advance(l);
            if (!is_at_end(l) && (peek_char(l) == '+' || peek_char(l) == '-')) advance(l);
            while (!is_at_end(l) && isdigit((unsigned char)peek_char(l))) advance(l);
        }
    }

    // suffixes
    while (!is_at_end(l)) {
        char c = peek_char(l);
        if (c == 'f' || c == 'F' || c == 'l' || c == 'L' ||
            c == 'u' || c == 'U' || c == 'z' || c == 'Z') {
            is_float = (c == 'f' || c == 'F' || c == 'l' || c == 'L');
            advance(l);
        } else break;
    }

    return make_token(l, is_float ? TOK_FLOAT_LIT : TOK_INT_LIT, sl, sc);
}

static Token tokenize_identifier_or_keyword(Lexer *l) {
    int sl = l->line, sc = l->column;
    while (!is_at_end(l)) {
        char c = peek_char(l);
        if (isalnum((unsigned char)c) || c == '_' || (unsigned char)c > 0x7F) advance(l);
        else break;
    }
    int length = (int)(l->current - l->start);

    // Check C keywords first
    for (int i = 0; c_keywords[i].keyword != NULL; i++) {
        int kw_len = (int)strlen(c_keywords[i].keyword);
        if (kw_len == length && memcmp(l->start, c_keywords[i].keyword, length) == 0)
            return make_token(l, TOK_KEYWORD, sl, sc);
    }
    // Check C++ keywords
    for (int i = 0; cpp_keywords[i].keyword != NULL; i++) {
        int kw_len = (int)strlen(cpp_keywords[i].keyword);
        if (kw_len == length && memcmp(l->start, cpp_keywords[i].keyword, length) == 0)
            return make_token(l, TOK_CPP_KEYWORD, sl, sc);
    }

    return make_token(l, TOK_IDENT, sl, sc);
}

static Token tokenize_preprocessor(Lexer *l) {
    int sl = l->line, sc = l->column;
    // Skip the # and any whitespace after it
    advance(l);
    while (!is_at_end(l) && (peek_char(l) == ' ' || peek_char(l) == '\t')) advance(l);

    // Read the directive name
    const char *dir_start = l->current;
    while (!is_at_end(l) && (isalnum((unsigned char)peek_char(l)) || peek_char(l) == '_')) advance(l);
    int dir_len = (int)(l->current - dir_start);

    TokenType type = TOK_PREPROCESSOR;
    if (dir_len == 7 && memcmp(dir_start, "include", 7) == 0) type = TOK_INCLUDE;
    else if (dir_len == 6 && memcmp(dir_start, "define", 6) == 0) type = TOK_DEFINE;
    else if (dir_len == 5 && memcmp(dir_start, "ifdef", 5) == 0) type = TOK_IFDEF;
    else if (dir_len == 6 && memcmp(dir_start, "ifndef", 6) == 0) type = TOK_IFNDEF;
    else if (dir_len == 4 && memcmp(dir_start, "endif", 4) == 0) type = TOK_ENDIF;
    else if (dir_len == 4 && memcmp(dir_start, "else", 4) == 0) type = TOK_ELSE_PP;
    else if (dir_len == 6 && memcmp(dir_start, "pragma", 6) == 0) type = TOK_PRAGMA;

    // Skip to end of line (preprocessor directives are line-based)
    while (!is_at_end(l) && peek_char(l) != '\n') {
        if (peek_char(l) == '\\' && peek_char_at(l, 1) == '\n') {
            advance(l); advance(l); // line continuation
        } else {
            advance(l);
        }
    }

    return make_token(l, type, sl, sc);
}

// ---- Public API ----

void lexer_init(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->current = source;
    lexer->start = source;
    lexer->line = 1;
    lexer->column = 1;
    lexer->start_line = 1;
    lexer->start_column = 1;
    lexer->in_block_comment = false;
    lexer->has_peeked = false;
}

Token lexer_next(Lexer *lexer) {
    if (lexer->has_peeked) {
        lexer->has_peeked = false;
        return lexer->peeked;
    }

    // If in a block comment from previous token, skip until we find */
    if (lexer->in_block_comment) {
        int sl = lexer->line, sc = lexer->column;
        lexer->in_block_comment = skip_block_comment(lexer);
        return make_token(lexer, lexer->in_block_comment ? TOK_ERROR : TOK_BLOCK_COMMENT, sl, sc);
    }

    skip_whitespace(lexer);

    if (is_at_end(lexer)) {
        return make_token(lexer, TOK_EOF, lexer->line, lexer->column);
    }

    lexer->start = lexer->current;
    lexer->start_line = lexer->line;
    lexer->start_column = lexer->column;

    char c = advance(lexer);

    // Preprocessor
    if (c == '#') return tokenize_preprocessor(lexer);

    // String literal
    if (c == '"') return tokenize_string(lexer);

    // Char literal
    if (c == '\'') return tokenize_char(lexer);

    // Number
    if (isdigit((unsigned char)c)) return tokenize_number(lexer);

    // Identifier or keyword
    if (isalpha((unsigned char)c) || c == '_' || (unsigned char)c > 0x7F)
        return tokenize_identifier_or_keyword(lexer);

    // Operators and punctuation
    int sl = lexer->start_line, sc = lexer->start_column;

    switch (c) {
        case '(': return make_token(lexer, TOK_LPAREN, sl, sc);
        case ')': return make_token(lexer, TOK_RPAREN, sl, sc);
        case '{': return make_token(lexer, TOK_LBRACE, sl, sc);
        case '}': return make_token(lexer, TOK_RBRACE, sl, sc);
        case '[': return make_token(lexer, TOK_LBRACKET, sl, sc);
        case ']': return make_token(lexer, TOK_RBRACKET, sl, sc);
        case ';': return make_token(lexer, TOK_SEMICOLON, sl, sc);
        case ',': return make_token(lexer, TOK_COMMA, sl, sc);
        case ':': return make_token(lexer, match(lexer, '>') ? TOK_RBRACKET : TOK_COLON, sl, sc);
        case '?': return make_token(lexer, TOK_QMARK, sl, sc);
        case '.': {
            if (match(lexer, '.')) {
                if (match(lexer, '.')) return make_token(lexer, TOK_ELLIPSIS, sl, sc);
                return make_token(lexer, TOK_DOT, sl, sc);
            }
            return make_token(lexer, TOK_DOT, sl, sc);
        }
        case '+': {
            if (match(lexer, '+')) return make_token(lexer, TOK_INC, sl, sc);
            if (match(lexer, '=')) return make_token(lexer, TOK_PLUS_ASSIGN, sl, sc);
            return make_token(lexer, TOK_PLUS, sl, sc);
        }
        case '-': {
            if (match(lexer, '-')) return make_token(lexer, TOK_DEC, sl, sc);
            if (match(lexer, '=')) return make_token(lexer, TOK_MINUS_ASSIGN, sl, sc);
            if (match(lexer, '>')) return make_token(lexer, TOK_ARROW, sl, sc);
            return make_token(lexer, TOK_MINUS, sl, sc);
        }
        case '*': {
            if (match(lexer, '=')) return make_token(lexer, TOK_STAR_ASSIGN, sl, sc);
            return make_token(lexer, TOK_STAR, sl, sc);
        }
        case '/': {
            if (match(lexer, '=')) return make_token(lexer, TOK_SLASH_ASSIGN, sl, sc);
            return make_token(lexer, TOK_SLASH, sl, sc);
        }
        case '%': {
            if (match(lexer, '=')) return make_token(lexer, TOK_PERCENT_ASSIGN, sl, sc);
            return make_token(lexer, TOK_PERCENT, sl, sc);
        }
        case '&': {
            if (match(lexer, '&')) return make_token(lexer, TOK_LOG_AND, sl, sc);
            if (match(lexer, '=')) return make_token(lexer, TOK_AMP_ASSIGN, sl, sc);
            return make_token(lexer, TOK_AMP, sl, sc);
        }
        case '|': {
            if (match(lexer, '|')) return make_token(lexer, TOK_LOG_OR, sl, sc);
            if (match(lexer, '=')) return make_token(lexer, TOK_PIPE_ASSIGN, sl, sc);
            return make_token(lexer, TOK_PIPE, sl, sc);
        }
        case '^': {
            if (match(lexer, '=')) return make_token(lexer, TOK_CARET_ASSIGN, sl, sc);
            return make_token(lexer, TOK_CARET, sl, sc);
        }
        case '~': return make_token(lexer, TOK_TILDE, sl, sc);
        case '!': {
            if (match(lexer, '=')) return make_token(lexer, TOK_NEQ, sl, sc);
            return make_token(lexer, TOK_LOG_NOT, sl, sc);
        }
        case '=': {
            if (match(lexer, '=')) return make_token(lexer, TOK_EQ, sl, sc);
            return make_token(lexer, TOK_ASSIGN, sl, sc);
        }
        case '<': {
            if (match(lexer, '=')) return make_token(lexer, TOK_LTE, sl, sc);
            if (match(lexer, '<')) {
                if (match(lexer, '=')) return make_token(lexer, TOK_LSHIFT_ASSIGN, sl, sc);
                return make_token(lexer, TOK_LSHIFT, sl, sc);
            }
            return make_token(lexer, TOK_LT, sl, sc);
        }
        case '>': {
            if (match(lexer, '=')) return make_token(lexer, TOK_GTE, sl, sc);
            if (match(lexer, '>')) {
                if (match(lexer, '=')) return make_token(lexer, TOK_RSHIFT_ASSIGN, sl, sc);
                return make_token(lexer, TOK_RSHIFT, sl, sc);
            }
            return make_token(lexer, TOK_GT, sl, sc);
        }
    }

    return error_token(lexer, sl, sc);
}

Token lexer_peek(Lexer *lexer) {
    if (!lexer->has_peeked) {
        lexer->peeked = lexer_next(lexer);
        lexer->has_peeked = true;
    }
    return lexer->peeked;
}

Token lexer_peek_next(Lexer *lexer) {
    // Save state
    Lexer saved = *lexer;
    // Get current
    lexer_next(lexer);
    // Get next
    Token t2 = lexer_next(lexer);
    // Restore and set peeked
    *lexer = saved;
    lexer->peeked = t2;
    lexer->has_peeked = true;
    return t2;
}

Token lexer_expect(Lexer *lexer, TokenType type, const char *message) {
    Token t = lexer_next(lexer);
    if (t.type != type) {
        fprintf(stderr, "Line %d, Col %d: expected %s, got %s (%.*s) - %s\n",
                t.line, t.column, token_type_name(type), token_type_name(t.type),
                t.length, t.start, message);
    }
    return t;
}

const char *token_type_name(TokenType type) {
    switch (type) {
        case TOK_INT_LIT:      return "integer";
        case TOK_FLOAT_LIT:    return "float";
        case TOK_CHAR_LIT:     return "char_literal";
        case TOK_STRING_LIT:   return "string_literal";
        case TOK_IDENT:        return "identifier";
        case TOK_KEYWORD:      return "keyword";
        case TOK_CPP_KEYWORD:  return "cpp_keyword";
        case TOK_PLUS:         return "+";
        case TOK_MINUS:        return "-";
        case TOK_STAR:         return "*";
        case TOK_SLASH:        return "/";
        case TOK_PERCENT:      return "%";
        case TOK_AMP:          return "&";
        case TOK_PIPE:         return "|";
        case TOK_CARET:        return "^";
        case TOK_TILDE:        return "~";
        case TOK_BANG:         return "!";
        case TOK_ASSIGN:       return "=";
        case TOK_EQ:           return "==";
        case TOK_NEQ:          return "!=";
        case TOK_LT:           return "<";
        case TOK_GT:           return ">";
        case TOK_LTE:          return "<=";
        case TOK_GTE:          return ">=";
        case TOK_LSHIFT:       return "<<";
        case TOK_RSHIFT:       return ">>";
        case TOK_LOG_AND:      return "&&";
        case TOK_LOG_OR:       return "||";
        case TOK_LOG_NOT:      return "!";
        case TOK_INC:          return "++";
        case TOK_DEC:          return "--";
        case TOK_ARROW:        return "->";
        case TOK_DOT:          return ".";
        case TOK_PLUS_ASSIGN:  return "+=";
        case TOK_MINUS_ASSIGN: return "-=";
        case TOK_STAR_ASSIGN:  return "*=";
        case TOK_SLASH_ASSIGN: return "/=";
        case TOK_PERCENT_ASSIGN:return "%=";
        case TOK_AMP_ASSIGN:   return "&=";
        case TOK_PIPE_ASSIGN:  return "|=";
        case TOK_CARET_ASSIGN: return "^=";
        case TOK_LSHIFT_ASSIGN:return "<<=";
        case TOK_RSHIFT_ASSIGN:return ">>=";
        case TOK_ADDR_OF:      return "&(address)";
        case TOK_DEREF:        return "*(deref)";
        case TOK_LPAREN:       return "(";
        case TOK_RPAREN:       return ")";
        case TOK_LBRACE:       return "{";
        case TOK_RBRACE:       return "}";
        case TOK_LBRACKET:     return "[";
        case TOK_RBRACKET:     return "]";
        case TOK_SEMICOLON:    return ";";
        case TOK_COMMA:        return ",";
        case TOK_COLON:        return ":";
        case TOK_QMARK:        return "?";
        case TOK_ELLIPSIS:     return "...";
        case TOK_PREPROCESSOR: return "#";
        case TOK_INCLUDE:      return "#include";
        case TOK_DEFINE:       return "#define";
        case TOK_IFDEF:        return "#ifdef";
        case TOK_IFNDEF:       return "#ifndef";
        case TOK_ENDIF:        return "#endif";
        case TOK_ELSE_PP:      return "#else";
        case TOK_PRAGMA:       return "#pragma";
        case TOK_NEWLINE:      return "\\n";
        case TOK_EOF:          return "EOF";
        case TOK_ERROR:        return "ERROR";
        case TOK_COMMENT:      return "comment";
        case TOK_BLOCK_COMMENT:return "block_comment";
        case TOK_WHITESPACE:   return "whitespace";
    }
    return "unknown";
}

bool token_is_keyword(Token token) {
    return token.type == TOK_KEYWORD || token.type == TOK_CPP_KEYWORD;
}

bool token_is_type(Token token) {
    if (token.type != TOK_KEYWORD) return false;
    return (token.length == 3 && memcmp(token.start, "int", 3) == 0) ||
           (token.length == 5 && memcmp(token.start, "float", 5) == 0) ||
           (token.length == 6 && memcmp(token.start, "double", 6) == 0) ||
           (token.length == 4 && memcmp(token.start, "char", 4) == 0) ||
           (token.length == 4 && memcmp(token.start, "void", 4) == 0) ||
           (token.length == 4 && memcmp(token.start, "long", 4) == 0) ||
           (token.length == 5 && memcmp(token.start, "short", 5) == 0) ||
           (token.length == 6 && memcmp(token.start, "signed", 6) == 0) ||
           (token.length == 8 && memcmp(token.start, "unsigned", 8) == 0) ||
           (token.length == 4 && memcmp(token.start, "bool", 4) == 0) ||
           (token.length == 5 && memcmp(token.start, "_Bool", 5) == 0) ||
           (token.length == 6 && memcmp(token.start, "struct", 6) == 0) ||
           (token.length == 5 && memcmp(token.start, "union", 5) == 0) ||
           (token.length == 4 && memcmp(token.start, "enum", 4) == 0);
}

bool token_is_operator(Token token) {
    return token.type >= TOK_PLUS && token.type <= TOK_DEREF;
}

// ---- Token stream ----

TokenStream tokenize_source(const char *source) {
    TokenStream stream = {NULL, 0, 0};
    stream.capacity = 1024;
    stream.tokens = malloc(stream.capacity * sizeof(Token));
    if (!stream.tokens) return stream;

    Lexer lexer;
    lexer_init(&lexer, source);

    for (;;) {
        Token t = lexer_next(&lexer);
        if (stream.count >= stream.capacity) {
            stream.capacity *= 2;
            Token *tmp = realloc(stream.tokens, stream.capacity * sizeof(Token));
            if (!tmp) { free(stream.tokens); stream.tokens = NULL; return stream; }
            stream.tokens = tmp;
        }
        stream.tokens[stream.count++] = t;
        if (t.type == TOK_EOF) break;
    }

    return stream;
}

void token_stream_free(TokenStream *stream) {
    if (stream->tokens) free(stream->tokens);
    stream->tokens = NULL;
    stream->count = 0;
}
