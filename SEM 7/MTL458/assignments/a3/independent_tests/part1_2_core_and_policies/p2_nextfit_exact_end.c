/* p2_nextfit_exact_end: next fit: a free block ending exactly at the position is not chosen; a coalesced free block containing the position is chosen at its start */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *d, *b2, *z1, *x, *y, *w, *v, *u;

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
    my_free(b);
    b2 = my_malloc((size_t)10);
    my_free(b2);
    my_free(a);
    my_set_policy(NEXT_FIT);
    z1 = my_malloc((size_t)10);
    my_heap_dump();
    my_free(c);
    x = my_malloc((size_t)10);
    my_free(x);
    y = my_malloc((size_t)10);
    my_heap_dump();
    w = my_malloc((size_t)5000);
    v = my_malloc((size_t)10);
    my_free(w);
    my_free(v);
    u = my_malloc((size_t)20);
    my_heap_dump();
    return 0;
}
