/* p2_nextfit_basic: next fit from 0:0; skips earlier holes; position inside a coalesced free block; wrap after the region end; growth when nothing fits */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *d, *e, *f, *g, *h, *i, *j;

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
    my_set_policy(NEXT_FIT);
    a = my_malloc((size_t)10);
    b = my_malloc((size_t)10);
    c = my_malloc((size_t)10);
    d = my_malloc((size_t)10);
    my_free(a);
    my_free(c);
    e = my_malloc((size_t)10);
    my_free(e);
    f = my_malloc((size_t)10);
    my_heap_dump();
    g = my_malloc((size_t)64800);
    h = my_malloc((size_t)10);
    i = my_malloc((size_t)10);
    j = my_malloc((size_t)10);
    my_heap_dump();
    return 0;
}
