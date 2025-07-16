#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <forge/array.h>
#include <forge/str.h>

#include "parser.h"
#include "kwds.h"
#include "lexer.h"

#define LP(l) l->hd && l->hd

static int
isty(token *t)
{
        const char *tys[] = TYPES;
        for (size_t i = 0; i < sizeof(tys)/sizeof(*tys); ++i) {
                if (!strcmp(tys[i], t->lx)
                    || !strcmp("const", t->lx)
                    || (t->lx[0] == '*' && !t->lx[1])) {
                        return 1;
                } else if (t->ty == TT_IDENTIFIER && t->n && t->n->ty == TT_IDENTIFIER) {
                        return 1;
                }
        }
        return 0;
}

char *
consume_function_body(lexer *lexer)
{
        forge_str content = forge_str_from("{");
        lexer_discard(lexer);
        int stack = 1;
        while (lexer->hd && stack != 0) {
                if (LP(lexer)->ty == TT_RCURLY) {
                        --stack;
                }
                else if (LP(lexer)->ty == TT_LCURLY) {
                        ++stack;
                }
                forge_str_concat(&content, lexer->hd->lx);
                forge_str_append(&content, ' ');
                lexer_discard(lexer);
        }
        return content.data;
}

char *
parse_line(lexer *lexer)
{
        forge_str res = forge_str_create();

        if (LP(lexer)->ty == TT_KEYWORD) {
                const char *s = lexer->hd->lx;
                if (!strcmp(s, KW_TYPEDEF) && !strcmp(lexer->hd->n->lx, KW_STRUCT)) {
                        lexer_discard(lexer);
                        lexer_discard(lexer);
                        forge_str_concat(&res, "typedef struct");
                        forge_str_concat(&res, consume_function_body(lexer));
                        forge_str_concat(&res, lexer_next(lexer)->lx);
                        forge_str_append(&res, ';');
                        return res.data;
                } else if (!strcmp(s, KW_STRUCT)) {
                        assert(0 && "unimplemented");
                } else {
                        assert(0 && "unimplemented");
                }
        }

        // Gather type
        while (LP(lexer) && isty(lexer->hd)) {
                forge_str_concat(&res, lexer->hd->lx);
                forge_str_append(&res, ' ');
                lexer_discard(lexer);
        }

        // No type, not a declaration, early return.
        if (res.len == 0) {
                lexer_discard(lexer);
                return NULL;
        }

        // Name of the function/identifier
        forge_str_append(&res, ' ');
        char *identifier = lexer_next(lexer)->lx;
        if (!strcmp(identifier, "main")) {
                while (LP(lexer)->ty != TT_LCURLY) lexer_discard(lexer);
                free(consume_function_body(lexer));
                forge_str_destroy(&res);
                return NULL;
        }
        forge_str_concat(&res, identifier);

        // Function
        if (LP(lexer)->ty == TT_LPAREN) {
                forge_str_append(&res, '(');
                lexer_discard(lexer);
                while (LP(lexer)->ty != TT_RPAREN) {
                        forge_str_concat(&res, lexer_next(lexer)->lx);
                        forge_str_append(&res, ' ');
                }
                forge_str_append(&res, ')');
                lexer_discard(lexer);
        }

        if (LP(lexer)->ty == TT_LCURLY) {
                free(consume_function_body(lexer));
        }

        char *extrn = forge_str_builder("extern ", res.data, ";", NULL);
        forge_str_destroy(&res);
        return extrn;
}

str_array
parse(lexer *lexer)
{
        str_array ar = dyn_array_empty(str_array);

        char *headers[] = {
                "stdio.h",
                "stdlib.h",
                "stdbool.h",
                "string.h",
                "unistd.h",
                "stdint.h",
                "stddef.h",
        };

        for (size_t i = 0; i < sizeof(headers)/sizeof(*headers); ++i) {
                dyn_array_append(ar, forge_str_builder("#include<", headers[i], ">", NULL));
        }

        while (LP(lexer)) {
                char *line = parse_line(lexer);
                if (line) {
                        printf("line: %s\n", line);
                        dyn_array_append(ar, line);
                }
        }

        return ar;
}
