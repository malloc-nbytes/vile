#include <string.h>

#include <forge/array.h>
#include <forge/lexer.h>
#include <forge/str.h>
#include <forge/err.h>

#include "parser.h"

#define LP(l) forge_lexer_peek(l, 0) && forge_lexer_peek(l, 0)
#define LPN(l, n) forge_lexer_peek(l, n) && forge_lexer_peek(l, n)

char *
parse_function(forge_lexer *lexer)
{
        if (forge_lexer_peek(lexer, 0)->ty == FORGE_TOKEN_TYPE_KEYWORD
            && !strcmp(forge_lexer_peek(lexer, 0)->lx, "static")) {
                return NULL;
        }

        forge_str line = forge_str_from("extern ");

        while (1) {
                forge_token *t = forge_lexer_next(lexer);
                if (!t) break;
                if (t->ty == FORGE_TOKEN_TYPE_SEMICOLON) break;
                if (t->ty == FORGE_TOKEN_TYPE_LEFT_CURLY) break;
                forge_str_append(&line, ' ');
                forge_str_concat(&line, t->lx);
        }

        forge_str_append(&line, ';');
        return line.data;
}

// TODO: support includes with quotes
char *
parse_include(forge_lexer *lexer)
{
        if (LPN(lexer, 1)->ty == FORGE_TOKEN_TYPE_IDENTIFIER && !strcmp(forge_lexer_peek(lexer, 1)->lx, "main")) {
                return NULL;
        }

        forge_str line = forge_str_create();

        while (1) {
                forge_token *t = forge_lexer_next(lexer);
                if (!t) break;
                if (t->ty == FORGE_TOKEN_TYPE_GREATERTHAN) break;
                forge_str_concat(&line, t->lx);
        }

        forge_str_append(&line, '>');

        return line.data;
}

char *
parse_stmt(forge_lexer *lexer)
{
        if (forge_lexer_peek(lexer, 2) &&
            (forge_lexer_peek(lexer, 0)->ty == FORGE_TOKEN_TYPE_KEYWORD
             || forge_lexer_peek(lexer, 0)->ty == FORGE_TOKEN_TYPE_IDENTIFIER)
            && (forge_lexer_peek(lexer, 1)->ty == FORGE_TOKEN_TYPE_KEYWORD
                || forge_lexer_peek(lexer, 1)->ty == FORGE_TOKEN_TYPE_IDENTIFIER)
            && (forge_lexer_peek(lexer, 2)->ty == FORGE_TOKEN_TYPE_LEFT_PARENTHESIS)) {
                return parse_function(lexer);
        }

        if (LP(lexer)->ty == FORGE_TOKEN_TYPE_HASH
                   && LPN(lexer, 1)->ty == FORGE_TOKEN_TYPE_IDENTIFIER
                   && !strcmp(forge_lexer_peek(lexer, 1)->lx, "include")) {
                return parse_include(lexer);
        }

        forge_lexer_discard(lexer);
        return NULL;
}

str_array
parse(forge_lexer *lexer)
{
        str_array ar = dyn_array_empty(str_array);

        while (LP(lexer)->ty != FORGE_TOKEN_TYPE_EOF) {
                char *line = parse_stmt(lexer);
                if (line) {
                        //printf("line: %s\n", line);
                        dyn_array_append(ar, line);
                }
        }

        return ar;
}
