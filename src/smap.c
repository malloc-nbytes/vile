#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "smap.h"

// TODO: Resize table if load value is big enough.

static __smap_node *
__smap_node_alloc(const char *k, void *v)
{
        __smap_node *n = (__smap_node *)malloc(sizeof(__smap_node));
        n->k = strdup(k);
        n->v = v;
        n->n = NULL;
        return n;
}

static unsigned
djb2(const char *s)
{
        unsigned hash = 5381;
        int c;

        while ((c = *s++))
                hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

        return hash;
}

smap
smap_create(void)
{
        return (smap) {
                .tbl = (__smap_node **)calloc(
                        SMAP_DEFAULT_TBL_CAPACITY,
                        sizeof(__smap_node *)
                ),
                .cap = SMAP_DEFAULT_TBL_CAPACITY,
                .len = 0,
                .sz = 0,
        };
}

void
smap_insert(smap *map, const char *k, void *v)
{
        unsigned idx = djb2(k) % map->cap;
        __smap_node *it = map->tbl[idx];
        __smap_node *prev = NULL;

        while (it) {
                if (!strcmp(it->k, k)) {
                        it->v = v;
                        return;
                }
                prev = it;
                it = it->n;
        }

        it = __smap_node_alloc(k, v);

        if (prev) {
                prev->n = it;
        } else {
                map->tbl[idx] = it;
                ++map->len;
        }

        ++map->sz;
}

int
smap_contains(const smap *map,
                    const char       *k)
{
        return smap_get(map, k) != NULL;
}

void *
smap_get(const smap *map,
               const char       *k)
{
        unsigned idx = djb2(k) % map->cap;
        __smap_node *it = map->tbl[idx];

        while (it) {
                if (!strcmp(it->k, k)) {
                        return it->v;
                }
                it = it->n;
        }

        return NULL;
}

void
smap_destroy(smap *map)
{
        for (size_t i = 0; i < map->cap; ++i) {
                __smap_node *it = map->tbl[i];
                while (it) {
                        __smap_node *tmp = it;
                        it = it->n;
                        free(tmp->k);
                        free(tmp);
                }
        }
        free(map->tbl);
        map->sz = map->len = map->cap = 0;
}

char **
smap_iter(const smap *map)
{
        char **keys = (char **)malloc(sizeof(char *) * (map->sz + 1));
        if (!keys) {
                return NULL;
        }

        size_t key_idx = 0;

        for (size_t i = 0; i < map->cap; ++i) {
                __smap_node *node = map->tbl[i];
                while (node) {
                        keys[key_idx] = node->k;
                        key_idx++;
                        node = node->n;
                }
        }

        keys[key_idx] = NULL;

        return keys;
}

size_t
smap_size(const smap *map)
{
        return map->sz;
}
