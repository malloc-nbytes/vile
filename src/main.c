#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <forge/io.h>
#include <forge/arg.h>
#include <forge/lexer.h>
#include <forge/str.h>
#include <forge/cstr.h>
#include <forge/rdln.h>
#include <forge/cmd.h>
#include <forge/err.h>

#include "parser.h"
#include "flags.h"

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
        str_array include_dirs;
} g_config = {
        .flags = 0x0000,
        .fp = NULL,
        .include_dirs = dyn_array_empty(str_array),
};

static const char *g_kwds[] = FORGE_LEXER_C_KEYWORDS;

forge_lexer
lex_file(const char *fp)
{
        assert(fp);

        char *src = forge_io_read_file_to_cstr(fp);

        forge_lexer lexer = forge_lexer_create((forge_lexer_config){
                .fp = fp,
                .src = src,
                .comment = {
                        .single      = "//",
                        .multi_start = "/*",
                        .multi_end   = "*/",
                },
                .kwds = (const char **)g_kwds,
                .bits = FORGE_LEXER_C_OPERATORS,
        });

        if (forge_lexer_has_err(&lexer)) {
                printf("%s:%zu:%zu: %s\n", lexer.fp, lexer.err.r, lexer.err.c, lexer.err.msg);
                exit(1);
        }

        free(src);
        return lexer;
}

forge_str
get_include_dirs(void)
{
        forge_str include_dirs = forge_str_create();
        for (size_t i = 0; i < g_config.include_dirs.len; ++i) {
                forge_str_concat(&include_dirs, " -I");
                forge_str_concat(&include_dirs, g_config.include_dirs.data[i]);
        }
        return include_dirs;
}

void
run(str_array *input_lines, str_array *user_lines)
{
        str_array final_lines = dyn_array_empty(str_array);
        for (size_t i = 0; i < input_lines->len; ++i) {
                dyn_array_append(final_lines, input_lines->data[i]);
        }
        for (size_t i = 0; i < user_lines->len; ++i) {
                dyn_array_append(final_lines, user_lines->data[i]);
        }

        forge_io_create_file(TMP_FILEPATH, 1);
        forge_io_write_lines(TMP_FILEPATH, (const char **)final_lines.data, final_lines.len);

        forge_str include_dirs = get_include_dirs();

        if (g_config.fp) {
                const char *basename = forge_io_basename(g_config.fp);

                char *cc_cmd = forge_cstr_builder("cc -fPIC -c ", g_config.fp, " -o /tmp/", basename, ".o",
                                                  include_dirs.len > 0 ? include_dirs.data : "", NULL);
                CMD(cc_cmd, {
                        free(cc_cmd);
                        goto bad;
                });
                char *ar_cmd = forge_cstr_builder("ar rcs /tmp/lib", basename, ".a /tmp/", basename, ".o", NULL);
                CMD(ar_cmd, {
                        free(cc_cmd);
                        free(ar_cmd);
                        goto bad;
                });
                char *cc2_cmd = forge_cstr_builder("cc " TMP_FILEPATH " -o /tmp/ViLe_bin_output -L/tmp/ -l", basename, NULL);
                CMD(cc2_cmd, {
                        free(cc2_cmd);
                        free(cc_cmd);
                        free(ar_cmd);
                        goto bad;
                });

                free(cc2_cmd);
                free(cc_cmd);
                free(ar_cmd);
        } else {
                char *cc_cmd = forge_cstr_builder("cc " TMP_FILEPATH " -o /tmp/ViLe_bin_output",
                                                  include_dirs.len > 0 ? include_dirs.data : "", NULL);
                CMD(cc_cmd, {
                        free(cc_cmd);
                        goto bad;
                });
                free(cc_cmd);
        }

        CMD("/tmp/ViLe_bin_output", goto bad);
        goto cleanup;

bad:
        printf("aborting...\n");

cleanup:
        forge_str_destroy(&include_dirs);
        dyn_array_free(final_lines);
}

char *
arg_expecteq(forge_arg *arg, const char *option)
{
        if (!arg->eq) {
                forge_err_wargs("option `%s` requires an equals (=)", option);
        }
        return strdup(arg->eq);
}

void
handle_args(forge_arg *it)
{
        while (it) {
                if (!it->h) {
                        if (g_config.fp) {
                                forge_err("Only zero or one file is supported at the moment.");
                        }
                        g_config.fp = forge_io_resolve_absolute_path(it->s);
                }
                else if (it->h == 1) {
                        if (it->s[0] == FLAG_1HY_INCLUDE) {
                                char op[] = {FLAG_1HY_INCLUDE, 0};
                                dyn_array_append(g_config.include_dirs,
                                                 forge_io_resolve_absolute_path(arg_expecteq(it, op)));
                        }
                }
                it = it->n;
        }
}

int
main(int argc, char **argv)
{
        forge_arg *arghd = forge_arg_alloc(argc, argv, 1);
        handle_args(arghd);
        forge_arg_free(arghd);

        forge_lexer lexer = {0};
        str_array input_lines = dyn_array_empty(str_array);

        if (g_config.fp) {
                lexer = lex_file(g_config.fp);
                input_lines = parse(&lexer);

                printf("Available Interface:\n");
                for (size_t i = 0; i < input_lines.len; ++i) {
                        forge_str line = forge_str_from(input_lines.data[i]);
                        char *s = forge_str_contains_substr(&line, " ", 0);
                        if (s) printf("%s\n", s);
                }
        }

        while (1) {
                str_array user_lines = dyn_array_empty(str_array);
                dyn_array_append(user_lines, strdup("int main(void) {"));

                while (1) {
                        char *input = forge_rdln("ViLe: ");
                        if (!strcmp(input, ":r") || !strcmp(input, ":run")) {
                                free(input);
                                break;
                        } else if (!strcmp(input, ":i")) {
                                free(input);
                                input = forge_rdln("#include ");
                                dyn_array_append(input_lines, forge_cstr_builder("#include ", input, NULL));
                                free(input);
                        } else if (!strcmp(input, ":q") || !strcmp(input, ":quit")) {
                                free(input);
                                goto done;
                        } else if (!strcmp(input, ":h") || !strcmp(input, ":help")) {
                                printf("Commands:\n");
                                printf("  :r, :run   - Run the code\n");
                                printf("  :i         - Add an include directive\n");
                                printf("  :q, :quit  - Quit the program\n");
                                printf("  :h, :help  - Show this help message\n");
                                free(input);
                        } else if (input) {
                                dyn_array_append(user_lines, input);
                        }
                }

                dyn_array_append(user_lines, strdup("}"));

                run(&input_lines, &user_lines);

                for (size_t i = 0; i < user_lines.len; ++i) {
                        free(user_lines.data[i]);
                }
                dyn_array_free(user_lines);
        }

done:
        if (g_config.fp) {
                forge_lexer_destroy(&lexer);
                for (size_t i = 0; i < input_lines.len; ++i) {
                        free(input_lines.data[i]);
                }
                dyn_array_free(input_lines);
        }
        return 0;
}
