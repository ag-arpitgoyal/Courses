/* p1_coalesce_prev_next: coalescing with next only, with both, with previous only */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *x, *y, *z;

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
    a = my_malloc((size_t)100);
    b = my_malloc((size_t)100);
    c = my_malloc((size_t)100);
    my_free(a);
    my_free(c);
    my_heap_dump();
    my_free(b);
    my_heap_dump();
    x = my_malloc((size_t)10);
    y = my_malloc((size_t)10);
    z = my_malloc((size_t)10);
    my_free(x);
    my_free(y);
    my_heap_dump();
    return 0;
}
