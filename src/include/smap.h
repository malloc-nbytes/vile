#ifndef SMAP_H_INCLUDED
#define SMAP_H_INCLUDED

#include <stddef.h>

#define SMAP_DEFAULT_TBL_CAPACITY 2048

typedef struct __smap_node {
        char *k; // owns the string
        void *v;
        struct __smap_node *n;
} __smap_node;

typedef struct {
        __smap_node **tbl;
        size_t len; // number of table entries
        size_t cap; // capacity of table
        size_t sz; // how many total nodes
} smap;

smap smap_create(void);
void smap_insert(smap *map, const char *k, void *v);
int smap_contains(const smap *map, const char *k);
void *smap_get(const smap *map, const char *k);
void smap_destroy(smap *map);
char **smap_iter(const smap *map);
size_t smap_size(const smap *map);

#endif // SMAP_H_INCLUDED
