/* p2_bestfit_regions: best fit across regions: a smaller block in a later region wins; exact fit; growth when nothing is free */
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
    a = my_malloc((size_t)65000);
    b = my_malloc((size_t)65000);
    c = my_malloc((size_t)10);
    my_set_policy(BEST_FIT);
    d = my_malloc((size_t)250);
    e = my_malloc((size_t)10);
    my_free(a);
    f = my_malloc((size_t)200);
    g = my_malloc((size_t)65000);
    h = my_malloc((size_t)10);
    my_heap_dump();
    return 0;
}
