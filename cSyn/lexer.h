#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdbool.h>

typedef enum {
    // Literals
    TOK_INT_LIT, TOK_FLOAT_LIT, TOK_CHAR_LIT, TOK_STRING_LIT,

    // Identifiers and keywords
    TOK_IDENT, TOK_KEYWORD,

    // C++ keywords
    TOK_CPP_KEYWORD,

    // Operators
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PERCENT,
    TOK_AMP, TOK_PIPE, TOK_CARET, TOK_TILDE, TOK_BANG,
    TOK_ASSIGN, TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LTE, TOK_GTE,
    TOK_LSHIFT, TOK_RSHIFT,
    TOK_LOG_AND, TOK_LOG_OR, TOK_LOG_NOT,
    TOK_INC, TOK_DEC,
    TOK_ARROW, TOK_DOT,
    TOK_PLUS_ASSIGN, TOK_MINUS_ASSIGN, TOK_STAR_ASSIGN, TOK_SLASH_ASSIGN,
    TOK_PERCENT_ASSIGN, TOK_AMP_ASSIGN, TOK_PIPE_ASSIGN, TOK_CARET_ASSIGN,
    TOK_LSHIFT_ASSIGN, TOK_RSHIFT_ASSIGN,
    TOK_ADDR_OF, TOK_DEREF,

    // Punctuation
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE, TOK_LBRACKET, TOK_RBRACKET,
    TOK_SEMICOLON, TOK_COMMA, TOK_COLON, TOK_QMARK, TOK_ELLIPSIS,

    // Preprocessor
    TOK_PREPROCESSOR, TOK_INCLUDE, TOK_DEFINE, TOK_IFDEF, TOK_IFNDEF,
    TOK_ENDIF, TOK_ELSE_PP, TOK_PRAGMA,

    // Special
    TOK_NEWLINE, TOK_EOF, TOK_ERROR, TOK_COMMENT, TOK_BLOCK_COMMENT,
    TOK_WHITESPACE
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int length;
    int line;
    int column;
} Token;

typedef struct {
    const char *source;
    const char *current;
    const char *start;
    int line;
    int column;
    int start_line;
    int start_column;
    bool in_block_comment;
    Token peeked;
    bool has_peeked;
} Lexer;

// Keyword tables
typedef struct {
    const char *keyword;
    TokenType type;
} KeywordEntry;

// Core lexer functions
void lexer_init(Lexer *lexer, const char *source);
Token lexer_next(Lexer *lexer);
Token lexer_peek(Lexer *lexer);
Token lexer_peek_next(Lexer *lexer);
Token lexer_expect(Lexer *lexer, TokenType type, const char *message);

// Token helpers
const char *token_type_name(TokenType type);
bool token_is_keyword(Token token);
bool token_is_type(Token token);
bool token_is_operator(Token token);

// Token stream (dynamic array)
typedef struct {
    Token *tokens;
    int count;
    int capacity;
} TokenStream;

TokenStream tokenize_source(const char *source);
void token_stream_free(TokenStream *stream);

#endif
