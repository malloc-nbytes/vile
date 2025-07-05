#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "grammar.h"
#include "array.h"
#define CIO_IMPL
#include "cio.h"

#include <forge/cmd.h>

#define C_REPL_FILE "dynamiC.c"
#define C_REPL_FILE_EXE "dynamiC-repl"

void
run_new_repl(void)
{
        (void)cmdout("cc -o " C_REPL_FILE_EXE " " C_REPL_FILE " -L. -ltest");
        printf("%s\n", cmdout("./dynamiC-repl"));
}

void
create_new_repl(const char *code,
                const char *main_body)
{
        char_array ar = dyn_array_empty(char_array);
        appendstr("#include<stdio.h>\n", &ar);
        appendstr("#include<stdlib.h>\n", &ar);
        appendstr(code, &ar);

        cio_create_file(C_REPL_FILE, 1);
        appendstr("\nint main(void){\n", &ar);
        appendstr(main_body, &ar);
        appendstr("\n}", &ar);
        dyn_array_append(ar, 0);

        cio_write_file("dynamiC.c", ar.data);
}

void
remove_repl_file(void)
{
        (void)remove(C_REPL_FILE);
        (void)remove(C_REPL_FILE_EXE);
}

int
main(int argc, char **argv)
{
        size_t N;
        char *src = cio_file_to_cstr_wnewlines("test.c", &N);
        lexer lexer = lex_file(src);
        //lexer_dump(&lexer);

        program p = create_program(&lexer);
        //dump_program(&p);

        atexit(remove_repl_file);

        const char *header_data = gen_data_from_program(&p);

        while (1) {
                printf(">>> ");
                char main_body[128] = {0};
                if (fgets(main_body, sizeof(main_body), stdin) == NULL) {
                        break;
                }
                if (!strcmp(main_body, "\n")) continue;
                main_body[strcspn(main_body, "\n")] = '\0';
                if (!strcmp(main_body, "@exit")) break;

                create_new_repl(header_data, main_body);
                run_new_repl();
        }

        return 0;
}
