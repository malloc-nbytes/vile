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
                }
        }
        return 0;
}

void
consume_function(lexer *lexer)
{
        lexer_discard(lexer);
        int stack = 1;
        while (lexer->hd && stack != 0) {
                if (LP(lexer)->ty == TT_RCURLY) {
                        --stack;
                }
                else if (LP(lexer)->ty == TT_LCURLY) {
                        ++stack;
                }
                lexer_discard(lexer);
        }
}

char *
parse_line(lexer *lexer)
{
        forge_str res = forge_str_create();

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

        forge_str_append(&res, ' ');
        forge_str_concat(&res, lexer_next(lexer)->lx);

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

        char *extrn = forge_str_builder("extern ", res.data, ";", NULL);
        forge_str_destroy(&res);

        if (LP(lexer)->ty == TT_LCURLY) {
                consume_function(lexer);
        }

        return extrn;
}

str_array
parse(lexer *lexer)
{
        str_array ar = dyn_array_empty(str_array);

        while (LP(lexer)) {
                char *line = parse_line(lexer);
                if (line) {
                        printf("line: %s\n", line);
                        dyn_array_append(ar, line);
                }
        }

        return ar;
}
