#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "grammar.h"
#include "array.h"
#define CIO_IMPL
#include "cio.h"

extern int sum(int, int);

int
main(int argc, char **argv)
{
        size_t N;
        char *src = cio_file_to_cstr_wnewlines("test.c", &N);
        lexer lexer = lex_file(src);
        lexer_dump(&lexer);

        program p = create_program(&lexer);
        dump_program(&p);

        return 0;
}
