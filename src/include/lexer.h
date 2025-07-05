#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

typedef enum {
        TT_EOF = 0,
        TT_IDENTIFIER,
        TT_STRLIT,
        TT_CHARLIT,
        TT_INTLIT,
        TT_KEYWORD,
        TT_TYPE,
        TT_LPAREN,
        TT_RPAREN,
        TT_ASERTISK,
        TT_SEMICOLON,
        TT_PERIOD,
        TT_COMMA,
        TT_HASH,
        TT_AMPERSAND,
        TT_LSQR,
        TT_RSQR,
        TT_LCURLY,
        TT_RCURLY,
        TT_BANG,
        TT_PLUS,
        TT_MINUS,
        TT_FORWARDSLASH,
        TT_GT,
        TT_LT,
        TT_EQ,
} token_type;

typedef struct token {
        char *lx;
        token_type ty;
        struct token *n;
} token;

typedef struct {
        token *hd;
        token *tl;
} lexer;

char *token_type_to_str(token_type ty);
lexer lex_file(char *src);
void lexer_dump(const lexer *lexer);

#endif // LEXER_H_INCLUDED
