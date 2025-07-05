#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "grammar.h"
#include "lexer.h"

#define LHD(l) l->hd && l->hd

static token *
expect(lexer *lexer, token_type ty)
{
        if (!lexer->hd || lexer->hd->ty != ty) {
                fprintf(stderr, "expected %s but got %s\n",
                        token_type_to_str(ty), token_type_to_str(lexer->hd->ty));
                exit(1);
        }
        token *it = lexer->hd;
        lexer->hd = lexer->hd->n;
        return it;
}

static void
discard(lexer *lexer)
{
        lexer->hd = lexer->hd->n;
}

static char *
get_type(lexer *lexer)
{
        char *ty = lexer->hd->lx;
        lexer->hd = lexer->hd->n;
        return ty;
}

static stmt *
parse_fun(lexer *lexer)
{
        parameter_ptr_array params = dyn_array_empty(parameter_ptr_array);

        char *rettype = get_type(lexer);
        char *name = expect(lexer, TT_IDENTIFIER)->lx;
        (void)expect(lexer, TT_LPAREN);
        while (LHD(lexer)->ty != TT_RPAREN) {
                char *ptype = get_type(lexer);
                char *name = expect(lexer, TT_IDENTIFIER)->lx;
                parameter *p = (parameter *)malloc(sizeof(parameter));
                p->type = ptype;
                p->name = name;
                dyn_array_append(params, p);
                if (LHD(lexer)->ty == TT_COMMA) {
                        (void)expect(lexer, TT_COMMA);
                }
        }
        (void)expect(lexer, TT_RPAREN);
        function *f = (function*)malloc(sizeof(function));
        f->rettype = rettype;
        f->name = name;
        f->params = params;
        f->base.ty = STMT_TYPE_FUNCTION;
        return (stmt*)f;
}

static void
dump_fun(const stmt *s)
{
        const function *f = (function*)s;
        printf("FUN %s %s (", f->rettype, f->name);
        for (size_t i = 0; i < f->params.len; ++i) {
                printf("%s %s", f->params.data[i]->type,
                       f->params.data[i]->name);
                if (i != f->params.len-1) {
                        printf(", ");
                }
        }
        printf(")\n");
}

void
dump_program(const program *p)
{
        for (size_t i = 0; i < p->stmts.len; ++i) {
                switch (p->stmts.data[i]->ty) {
                case STMT_TYPE_FUNCTION: {
                        dump_fun(p->stmts.data[i]);
                } break;
                default: {
                        fprintf(stderr, "unknown stmt type %d\n",
                                (int)p->stmts.data[i]->ty);
                        exit(1);
                } break;
                }
        }
}

program
create_program(lexer *lexer)
{
        stmt_ptr_array ar = dyn_array_empty(stmt_ptr_array);
        while (LHD(lexer)->ty != TT_EOF) {
                if ((LHD(lexer)->ty == TT_TYPE || lexer->hd->ty == TT_IDENTIFIER)
                    && lexer->hd->n && lexer->hd->n->ty == TT_IDENTIFIER
                    && lexer->hd->n->n && lexer->hd->n->n->ty == TT_LPAREN) {
                        dyn_array_append(ar, parse_fun(lexer));
                } else {
                        discard(lexer);
                }
        }
        return (program) {
                .stmts = ar,
        };
}
