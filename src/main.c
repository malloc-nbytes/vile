#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <forge/array.h>
#include <forge/io.h>
#include <forge/cmd.h>
#include <forge/rdln.h>
#include <forge/ctrl.h>
#include <forge/arg.h>
#include <forge/str.h>

#include "lexer.h"
#include "parser.h"

#define HELP  ":h"
#define HELP2 ":help"
#define QUIT  ":q"
#define QUIT2 ":quit"
#define RUN   ":r"
#define RUN2  ":run"

#define TMP_FILEPATH "/tmp/.vile.c"

#define CMD(c, b) if (!cmd(c)) b

struct {
        uint32_t flags;
        char *fp;
} g_config = {
        .flags = 0x0000,
        .fp = NULL,
};

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

        char *cc_cmd = forge_str_builder("cc -c ", g_config.fp, " -o /tmp/", g_config.fp, ".o", NULL);
        CMD(cc_cmd, {
                free(cc_cmd);
                goto bad;
        });
        char *ar_cmd = forge_str_builder("ar rcs /tmp/lib", g_config.fp, ".a /tmp/", g_config.fp, ".o", NULL);
        CMD(ar_cmd, {
                free(cc_cmd);
                free(ar_cmd);
                goto bad;
        });
        char *cc2_cmd = forge_str_builder("cc " TMP_FILEPATH " -o /tmp/ViLe_bin_output -L/tmp/ -l", g_config.fp, NULL);
        CMD(cc2_cmd, {
                free(cc2_cmd);
                free(cc_cmd);
                free(ar_cmd);
                goto bad;
        });

        free(cc2_cmd);
        free(cc_cmd);
        free(ar_cmd);

        CMD("/tmp/ViLe_bin_output", goto bad);
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
        if (argv[1][0] == '.' && argv[1][1] == '/') {
                g_config.fp = &argv[1][2];
        } else {
                g_config.fp = argv[1];
        }

        size_t N;
        char *src = forge_io_read_file_to_cstr(g_config.fp);
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

        for (size_t i = 0; i < lines.len; ++i) {
                free(lines.data[i]);
        }
        dyn_array_free(lines);
        return 0;
}
