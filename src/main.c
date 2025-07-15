#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "forge/array.h"
#include "forge/io.h"

int
main(int argc, char **argv)
{
        size_t N;
        char *src = forge_io_read_file_to_cstr("test.c");
        lexer lexer = lex_file(src);
        //lexer_dump(&lexer);

        (void)parse(&lexer);

        return 0;
}
