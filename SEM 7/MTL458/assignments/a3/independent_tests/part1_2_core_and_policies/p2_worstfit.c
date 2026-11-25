/* p2_worstfit: worst fit: largest block across regions, ties -> earliest region, exact fit, heap grows when the largest block is too small */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *d, *e, *f, *g, *h, *i;

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
    a = my_malloc((size_t)65464);
    b = my_malloc((size_t)10);
    my_free(a);
    my_set_policy(WORST_FIT);
    c = my_malloc((size_t)10);
    d = my_malloc((size_t)10);
    e = my_malloc((size_t)10);
    f = my_malloc((size_t)10);
    g = my_malloc((size_t)65200);
    h = my_malloc((size_t)65200);
    i = my_malloc((size_t)100);
    my_heap_dump();
    return 0;
}
