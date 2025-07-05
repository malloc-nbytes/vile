#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"
#include "kwds.h"
#include "smap.h"

static smap g_ops;

static void
initops(void)
{
        g_ops = smap_create();
        static token_type eof = TT_EOF;
        static token_type identifier = TT_IDENTIFIER;
        static token_type strlit = TT_STRLIT;
        static token_type charlit = TT_CHARLIT;
        static token_type intlit = TT_INTLIT;
        static token_type keyword = TT_KEYWORD;
        static token_type type = TT_TYPE;
        static token_type lparen = TT_LPAREN;
        static token_type rparen = TT_RPAREN;
        static token_type asertisk = TT_ASERTISK;
        static token_type semicolon = TT_SEMICOLON;
        static token_type period = TT_PERIOD;
        static token_type comma = TT_COMMA;
        static token_type hash = TT_HASH;
        static token_type ampersand = TT_AMPERSAND;
        static token_type lsqr = TT_LSQR;
        static token_type rsqr = TT_RSQR;
        static token_type lcurly = TT_LCURLY;
        static token_type rcurly = TT_RCURLY;
        static token_type bang = TT_BANG;
        static token_type plus = TT_PLUS;
        static token_type minus = TT_MINUS;
        static token_type forwardslash = TT_FORWARDSLASH;
        static token_type gt = TT_GT;
        static token_type lt = TT_LT;
        static token_type eq = TT_EQ;
        smap_insert(&g_ops, "(", &lparen);
        smap_insert(&g_ops, ")", &rparen);
        smap_insert(&g_ops, "*", &asertisk);
        smap_insert(&g_ops, ";", &semicolon);
        smap_insert(&g_ops, ".", &period);
        smap_insert(&g_ops, ",", &comma);
        smap_insert(&g_ops, "#", &hash);
        smap_insert(&g_ops, "&", &ampersand);
        smap_insert(&g_ops, "[", &lsqr);
        smap_insert(&g_ops, "]", &rsqr);
        smap_insert(&g_ops, "{", &lcurly);
        smap_insert(&g_ops, "}", &rcurly);
        smap_insert(&g_ops, "!", &bang);
        smap_insert(&g_ops, "+", &plus);
        smap_insert(&g_ops, "-", &minus);
        smap_insert(&g_ops, "/", &forwardslash);
        smap_insert(&g_ops, "<", &lt);
        smap_insert(&g_ops, ">", &gt);
        smap_insert(&g_ops, "=", &eq);
}

static token *
token_alloc(char      *st,
            size_t     n,
            token_type ty)
{
        token *t = (token *)malloc(sizeof(token));
        t->lx = strndup(st, n);
        t->ty = ty;
        t->n = NULL;
        return t;
}

static size_t
consume_while(char *st,
              int (*pred)(int))
{
        size_t i = 0;
        int skip = 0;
        while (st[i]) {
                if (!pred(st[i]) && !skip) {
                        return i;
                } else if (st[i] == '\\') {
                        skip = 1;
                } else {
                        skip = 0;
                }
                ++i;
        }
}

static int
isident(int c)
{
        return isalnum(c) || (char)c == '_';
}

static int
iskw(char *s)
{
        const char *kwds[] = KWDS;
        for (size_t i = 0; i < sizeof(kwds)/sizeof(*kwds); ++i) {
                if (!strcmp(s, kwds[i])) {
                        return 1;
                }
        }
        return 0;
}

static int
isty(char *s)
{
        const char *types[] = TYPES;
        for (size_t i = 0; i < sizeof(types)/sizeof(*types); ++i) {
                if (!strcmp(s, types[i])) {
                        return 1;
                }
        }
        return 0;
}

static void
lexer_append(lexer *lexer, token *t)
{
        if (!lexer->hd && !lexer->tl) {
                lexer->hd = lexer->tl = t;
        } else {
                token *tmp = lexer->tl;
                lexer->tl = t;
                tmp->n = lexer->tl;
        }
}

char *
token_type_to_str(token_type ty)
{
        switch (ty) {
        case TT_EOF: return "TT_EOF";
        case TT_IDENTIFIER: return "TT_IDENTIFIER";
        case TT_STRLIT: return "TT_STRLIT";
        case TT_CHARLIT: return "TT_CHARLIT";
        case TT_INTLIT: return "TT_INTLIT";
        case TT_KEYWORD: return "TT_KEYWORD";
        case TT_TYPE: return "TT_TYPE";
        case TT_LPAREN: return "TT_LPAREN";
        case TT_RPAREN: return "TT_RPAREN";
        case TT_ASERTISK: return "TT_ASERTISK";
        case TT_SEMICOLON: return "TT_SEMICOLON";
        case TT_PERIOD: return "TT_PERIOD";
        case TT_COMMA: return "TT_COMMA";
        case TT_HASH: return "TT_HASH";
        case TT_AMPERSAND: return "TT_AMPERSAND";
        case TT_LSQR: return "TT_LSQR";
        case TT_RSQR: return "TT_RSQR";
        case TT_LCURLY: return "TT_LCURLY";
        case TT_RCURLY: return "TT_RCURLY";
        case TT_BANG: return "TT_BANG";
        case TT_PLUS: return "TT_PLUS";
        case TT_MINUS: return "TT_MINUS";
        case TT_FORWARDSLASH: return "TT_FORWARDSLASH";
        case TT_GT: return "TT_GT";
        case TT_LT: return "TT_LT";
        default: {
                assert(0 && "token_type_to_str(): unhandled token");
        } break;
        }
        return NULL; // unreachable
}

void
lexer_dump(const lexer *lexer)
{
        token *it = lexer->hd;
        while (it) {
                printf("Token: < %s ... %s >\n",
                       it->lx, token_type_to_str(it->ty));
                it = it->n;
        }
}

int
isstr(int c)
{
        return (char)c != '"';
}

lexer
lex_file(char *src)
{
        initops();

        lexer lexer = {
                .hd = NULL,
                .tl = NULL,
        };

        size_t i = 0;
        while (src[i]) {
                char c = src[i];
                if (c == ' ' || c == '\n' || c == '\t') {
                        ++i; // Skip whitespace
                } else if (c == '"') {
                        size_t len = consume_while(src + i + 1, isstr); // Consume until closing quote
                        if (src[i + len + 1] == '"') {
                                lexer_append(&lexer, token_alloc(src + i + 1, len, TT_STRLIT));
                                i += len + 2; // Skip quotes and content
                        } else {
                                fprintf(stderr, "Error: Unterminated string literal\n");
                                break;
                        }
                } else if (c == '\'') {
                        size_t len = consume_while(src + i + 1, isprint); // Consume until closing quote
                        if (src[i + len + 1] == '\'') {
                                lexer_append(&lexer, token_alloc(src + i + 1, len, TT_CHARLIT));
                                i += len + 2; // Skip quotes and content
                        } else {
                                fprintf(stderr, "Error: Unterminated character literal\n");
                                break;
                        }
                } else if (isalpha(c) || c == '_') {
                        size_t len = consume_while(src + i, isident);
                        token *t = token_alloc(src + i, len, TT_IDENTIFIER);
                        if (iskw(t->lx)) {
                                t->ty = TT_KEYWORD;
                        } else if (isty(t->lx)) {
                                t->ty = TT_TYPE;
                        }
                        lexer_append(&lexer, t);
                        i += len;
                } else if (isdigit(c)) {
                        size_t len = consume_while(src + i, isdigit);
                        lexer_append(&lexer, token_alloc(src + i, len, TT_INTLIT));
                        i += len;
                } else {
                        char op[2] = {c, '\0'};
                        token_type *ty = smap_get(&g_ops, op);
                        if (ty) {
                                lexer_append(&lexer, token_alloc(src + i, 1, *ty));
                                i += 1;
                        } else {
                                // Handle unrecognized character
                                fprintf(stderr, "Error: Unrecognized character '%c'\n", c);
                                i += 1;
                        }
                }
        }

        // Append EOF token
        lexer_append(&lexer, token_alloc("EOF", 3, TT_EOF));
        return lexer;
}
