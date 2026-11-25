/* p2_nextfit_pos_any_policy: the next-fit position moves after allocations made under BEST_FIT, WORST_FIT and FIRST_FIT; policy switched between calls */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *d, *e, *f, *g, *h, *x1, *x2, *x3, *x4, *x5, *x6;

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
    e = my_malloc((size_t)10);
    f = my_malloc((size_t)10);
    g = my_malloc((size_t)10);
    h = my_malloc((size_t)10);
    my_free(b);
    my_free(d);
    my_free(f);
    my_set_policy(BEST_FIT);
    x1 = my_malloc((size_t)81);
    my_set_policy(NEXT_FIT);
    x2 = my_malloc((size_t)82);
    my_free(x1);
    my_set_policy(WORST_FIT);
    x3 = my_malloc((size_t)83);
    my_set_policy(NEXT_FIT);
    x4 = my_malloc((size_t)84);
    my_set_policy(FIRST_FIT);
    x5 = my_malloc((size_t)85);
    my_set_policy(NEXT_FIT);
    x6 = my_malloc((size_t)86);
    my_heap_dump();
    return 0;
}
