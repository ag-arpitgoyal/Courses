/* p1_heap_order: first fit follows heap order (region number), not addresses */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *d, *e, *f;

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
    b = my_malloc((size_t)65464);
    c = my_malloc((size_t)10);
    my_free(b);
    my_free(a);
    d = my_malloc((size_t)10);
    e = my_malloc((size_t)65400);
    f = my_malloc((size_t)20);
    my_heap_dump();
    return 0;
}
