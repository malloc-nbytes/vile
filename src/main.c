#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "forge/array.h"
#include "forge/io.h"
#include "forge/cmd.h"

#include "lexer.h"
#include "parser.h"

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

int
main(int argc, char **argv)
{
        size_t N;
        char *src = forge_io_read_file_to_cstr("test.c");
        lexer lexer = lex_file(src);

        str_array lines = parse(&lexer);
        str_array user_lines = dyn_array_empty(str_array);
        dyn_array_append(lines, strdup("int main(void) {"));

        while (1) {
                char buf[256] = {0};
                printf("[ViLe]: ");
                if (fgets(buf, sizeof(buf), stdin) == NULL) {
                        // Handle EOF or error
                        break;
                }

                // Remove trailing newline
                size_t len = strlen(buf);
                if (len > 0 && buf[len - 1] == '\n') {
                        buf[len - 1] = '\0';
                        len--;
                } else if (len == sizeof(buf) - 1) {
                        // Input was truncated; clear input stream
                        int c;
                        while ((c = getchar()) != '\n' && c != EOF);
                }

                if (!strcmp(buf, ":eof")) {
                        dyn_array_append(user_lines, strdup("}"));
                        run_file(&lines, &user_lines);
                } else if (!strcmp(buf, ":q") || !strcmp(buf, ":quit")) {
                        break;
                } else {
                        dyn_array_append(user_lines, strdup(buf));
                }
        }

        cmd("rm ./ViLe_bin_output");
        cmd("rm ./" TMP_FILEPATH);

        for (size_t i = 0; i < lines.len; ++i) {
                free(lines.data[i]);
        }
        dyn_array_free(lines);
        return 0;
}
