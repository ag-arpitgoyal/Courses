/* p1_region_tail: split rule on region tails (remainder 112/128/144/0), B+32 == 65536 fits one unit, B+32 == 65552 needs 131072; later reuse */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *d, *e, *f, *g, *h;

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
    a = my_malloc((size_t)65352);
    b = my_malloc((size_t)65336);
    c = my_malloc((size_t)65320);
    d = my_malloc((size_t)65464);
    e = my_malloc((size_t)65465);
    f = my_malloc((size_t)89);
    g = my_malloc((size_t)10);
    my_heap_dump();
    my_free(b);
    my_free(g);
    my_free(a);
    h = my_malloc((size_t)65464);
    my_heap_dump();
    return 0;
}
