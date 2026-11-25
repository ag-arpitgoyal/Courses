/* p1_growth_sizes: growth: region sizes are the smallest multiple of 65536 >= B+32 (65536, 131072 exact, 196608, 327680); growth only when no block fits */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *d, *e, *f, *g;

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
    b = my_malloc((size_t)65464);
    c = my_malloc((size_t)65465);
    d = my_malloc((size_t)131000);
    e = my_malloc((size_t)131016);
    f = my_malloc((size_t)300000);
    g = my_malloc((size_t)65480);
    my_heap_dump();
    return 0;
}
