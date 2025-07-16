#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <forge/array.h>
#include <forge/io.h>
#include <forge/cmd.h>
#include <forge/rdln.h>
#include <forge/ctrl.h>

#include "lexer.h"
#include "parser.h"

#define HELP  ":h"
#define HELP2 ":help"
#define QUIT  ":q"
#define QUIT2 ":quit"
#define RUN   ":r"
#define RUN2  ":run"

#define TMP_FILEPATH ".vile.c"

#define CMD(c, b) if (!cmd(c)) b

void
run_file(const str_array *lines,
         str_array       *user_lines)
{
        str_array final_lines = dyn_array_empty(str_array);
        for (size_t i = 0; i < lines->len; ++i) {
                dyn_array_append(final_lines, lines->data[i]);
        }
        for (size_t i = 0; i < user_lines->len; ++i) {
                dyn_array_append(final_lines, user_lines->data[i]);
        }

        forge_io_create_file(TMP_FILEPATH, 1);
        forge_io_write_lines(TMP_FILEPATH, (const char **)final_lines.data, final_lines.len);

        CMD("cc -c test.c -o test.o", goto bad);
        CMD("ar rcs libtest.a test.o", goto bad);
        CMD("cc " TMP_FILEPATH " -o ViLe_bin_output -L. -ltest", goto bad);
        CMD("./ViLe_bin_output", goto bad);
        goto cleanup;

 bad:
        printf("aborting...\n");

 cleanup:
        for (size_t i = 0; i < user_lines->len; ++i) {
                free(user_lines->data[i]);
        }
        dyn_array_clear(*user_lines);
        dyn_array_free(final_lines);
}

void
usage(void)
{
        printf("Usage: ViLe <file.c>\n");
        exit(0);
}

int
main(int argc, char **argv)
{
        if (argc <= 1) { usage(); }

        size_t N;
        char *src = forge_io_read_file_to_cstr("test.c");
        lexer lexer = lex_file(src);

        str_array lines = parse(&lexer);
        str_array user_lines = dyn_array_empty(str_array);
        dyn_array_append(lines, strdup("int main(void) {"));

        while (1) {
                char *buf = forge_rdln("[ViLe]: ");

                if (!strcmp(buf, RUN) || !strcmp(buf, RUN2)) {
                        dyn_array_append(user_lines, strdup("}"));
                        run_file(&lines, &user_lines);
                } else if (!strcmp(buf, QUIT) || !strcmp(buf, QUIT2)) {
                        break;
                } else {
                        dyn_array_append(user_lines, strdup(buf));
                }

                free(buf);
        }

        cmd("rm ./ViLe_bin_output");
        cmd("rm ./" TMP_FILEPATH);

        for (size_t i = 0; i < lines.len; ++i) {
                free(lines.data[i]);
        }
        dyn_array_free(lines);
        return 0;
}
