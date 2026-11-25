/* p1_split_remainders: splitting threshold: remainders 112 (no split), 128 (split), 144 (split), 16 (no split), exact fits of split leftovers */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *g1, *b, *g2, *c, *g3, *x1, *x2, *x3, *x4, *x5, *x6;

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
    a = my_malloc((size_t)200);
    g1 = my_malloc((size_t)10);
    b = my_malloc((size_t)216);
    g2 = my_malloc((size_t)10);
    c = my_malloc((size_t)232);
    g3 = my_malloc((size_t)10);
    my_free(a);
    my_free(b);
    my_free(c);
    my_heap_dump();
    x1 = my_malloc((size_t)1);
    x2 = my_malloc((size_t)88);
    x3 = my_malloc((size_t)50);
    x4 = my_malloc((size_t)80);
    x5 = my_malloc((size_t)89);
    my_heap_dump();
    my_free(x1);
    x6 = my_malloc((size_t)184);
    my_heap_dump();
    return 0;
}
