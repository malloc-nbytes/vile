#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

#include "lexer.h"
#include "array.h"

typedef enum {
        STMT_TYPE_FUNCTION = 0,
} stmt_type;

typedef struct {
        stmt_type ty;
} stmt;

DYN_ARRAY_TYPE(stmt *, stmt_ptr_array);

typedef struct {
        char *type;
        char *name;
} parameter;

DYN_ARRAY_TYPE(parameter *, parameter_ptr_array);

typedef struct {
        stmt base;
        char *rettype;
        char *name;
        parameter_ptr_array params;
} function;

typedef struct {
        stmt_ptr_array stmts;
} program;

program create_program(lexer *lexer);
void dump_program(const program *p);

#endif // GRAMMAR_H_INCLUDED
