/* p1_coalesce: coalescing: no neighbour free, both neighbours, next only (with the tail), both including the tail */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *d, *e;

static void chk(const char *name, const char *p, size_t n, unsigned char t)
{
    size_t i;
    for (i = 0; i < n; i++)
        if ((unsigned char)p[i] != t) { printf("%s bad\n", name); return; }
    printf("%s ok\n", name);
}

int main(void)
{
    (void)chk;
    a = my_malloc((size_t)10);
    b = my_malloc((size_t)10);
    c = my_malloc((size_t)10);
    d = my_malloc((size_t)10);
    e = my_malloc((size_t)10);
    my_free(b);
    my_free(d);
    my_heap_dump();
    my_free(c);
    my_heap_dump();
    my_free(a);
    my_free(e);
    my_heap_dump();
    return 0;
}
