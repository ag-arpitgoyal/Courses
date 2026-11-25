/* p2_nextfit_wrap_before: next fit: growth moves the position into the new region; only fitting block lies before the position -> wrap to the start */
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
    my_set_policy(NEXT_FIT);
    a = my_malloc((size_t)1000);
    b = my_malloc((size_t)10);
    c = my_malloc((size_t)10);
    my_free(a);
    d = my_malloc((size_t)64300);
    e = my_malloc((size_t)1000);
    f = my_malloc((size_t)1000);
    g = my_malloc((size_t)500);
    my_heap_dump();
    return 0;
}
