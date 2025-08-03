#include <assert.h>
#include <string.h>

#include <forge/array.h>
#include <forge/lexer.h>
#include <forge/str.h>
#include <forge/err.h>

#include "parser.h"

#define LP(l) forge_lexer_peek(l, 0) && forge_lexer_peek(l, 0)
#define LPN(l, n) forge_lexer_peek(l, n) && forge_lexer_peek(l, n)

void
skipblk(forge_lexer *lexer)
{
        assert(forge_lexer_expect(lexer, FORGE_TOKEN_TYPE_LEFT_CURLY));
        int stack = 1;
        while (stack) {
                forge_token *t = forge_lexer_peek(lexer, 0);
                if (t->ty == FORGE_TOKEN_TYPE_LEFT_CURLY) ++stack;
                if (t->ty == FORGE_TOKEN_TYPE_RIGHT_CURLY) --stack;
                forge_lexer_discard(lexer);
        }
}

char *
parse_function(forge_lexer *lexer)
{
        if (forge_lexer_peek(lexer, 0)->ty == FORGE_TOKEN_TYPE_KEYWORD
            && !strcmp(forge_lexer_peek(lexer, 0)->lx, "static")) {
                return NULL;
        }

        if (!strcmp(forge_lexer_peek(lexer, 1)->lx, "main")) {
                forge_err_wargs("file `%s` contains an entrypoint, this needs to be removed.", lexer->fp);
                //while (LP(lexer)->ty != FORGE_TOKEN_TYPE_LEFT_CURLY) {
                //        forge_lexer_discard(lexer);
                //}
                //skipblk(lexer);
                //return NULL;
        }

        forge_str line = forge_str_from("extern ");

        while (1) {
                forge_token *t = forge_lexer_peek(lexer, 0);
                if (!t) break;
                if (t->ty == FORGE_TOKEN_TYPE_SEMICOLON) break;
                if (t->ty == FORGE_TOKEN_TYPE_LEFT_CURLY) break;
                forge_str_append(&line, ' ');
                forge_str_concat(&line, t->lx);
                forge_lexer_discard(lexer);
        }

        if (forge_lexer_peek(lexer, 0)->ty == FORGE_TOKEN_TYPE_LEFT_CURLY) {
                skipblk(lexer);
        }

        forge_str_append(&line, ';');
        return line.data;
}

char *
parse_struct(forge_lexer *lexer)
{
        forge_str res = forge_str_create();

        while (LP(lexer)->ty != FORGE_TOKEN_TYPE_LEFT_CURLY) {
                forge_str_concat(&res, forge_lexer_next(lexer)->lx);
                forge_str_append(&res, ' ');
        }

        int stack = 1;

        while (stack != 0) {
                forge_str_concat(&res, forge_lexer_next(lexer)->lx);
                forge_str_append(&res, ' ');
                if (LP(lexer)->ty == FORGE_TOKEN_TYPE_LEFT_CURLY) {
                        ++stack;
                } else if (LP(lexer)->ty == FORGE_TOKEN_TYPE_RIGHT_CURLY) {
                        --stack;
                }
        }
        forge_str_concat(&res, forge_lexer_next(lexer)->lx);

        while (LP(lexer)->ty != FORGE_TOKEN_TYPE_SEMICOLON) {
                forge_str_concat(&res, forge_lexer_next(lexer)->lx);
        }
        forge_str_concat(&res, forge_lexer_next(lexer)->lx);

        return res.data;
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

int
is_structure(const forge_lexer *lexer,
             const char        *structure_type)
{
        forge_token *t1 = forge_lexer_peek(lexer, 0);
        forge_token *t2 = forge_lexer_peek(lexer, 1);
        forge_token *t3 = forge_lexer_peek(lexer, 2);

        if (!t1 || !t2) {
                return 0;
        }

        return (!strcmp(t1->lx, "typedef") || !strcmp(t1->lx, structure_type))
                && (!strcmp(t2->lx, structure_type) || t2->ty == FORGE_TOKEN_TYPE_IDENTIFIER)
                && ((!t3 || t3->ty != FORGE_TOKEN_TYPE_IDENTIFIER) || (t3 && t3->ty == FORGE_TOKEN_TYPE_IDENTIFIER));
}

char *
parse_stmt(forge_lexer *lexer)
{
        // Function
        if (forge_lexer_peek(lexer, 2) &&
            (!strcmp(forge_lexer_peek(lexer, 0)->lx, "struct")
             && forge_lexer_peek(lexer, 1)->ty == FORGE_TOKEN_TYPE_IDENTIFIER
             && forge_lexer_peek(lexer, 2)->ty == FORGE_TOKEN_TYPE_IDENTIFIER)
            || (forge_lexer_peek(lexer, 0)->ty == FORGE_TOKEN_TYPE_KEYWORD
             || forge_lexer_peek(lexer, 0)->ty == FORGE_TOKEN_TYPE_IDENTIFIER)
            && (forge_lexer_peek(lexer, 1)->ty == FORGE_TOKEN_TYPE_KEYWORD
                || forge_lexer_peek(lexer, 1)->ty == FORGE_TOKEN_TYPE_IDENTIFIER)
            && (forge_lexer_peek(lexer, 2)->ty == FORGE_TOKEN_TYPE_LEFT_PARENTHESIS)) {
                return parse_function(lexer);
        }

        // Structure
        if (is_structure(lexer, "struct")) {
                return parse_struct(lexer);
        } else if (is_structure(lexer, "enum")) {
                return parse_struct(lexer);
        } else if (is_structure(lexer, "union")) {
                return parse_struct(lexer);
        }

        // Include directive
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
