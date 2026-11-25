/* p2_bestfit: best fit: smallest fitting block, ties -> earliest, unsplit small remainders, exact fit of a split leftover */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *h1, *g1, *h2, *g2, *h3, *g3, *h4, *g4, *x1, *x2, *x3, *x4, *x5;

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
    h1 = my_malloc((size_t)400);
    g1 = my_malloc((size_t)10);
    h2 = my_malloc((size_t)200);
    g2 = my_malloc((size_t)10);
    h3 = my_malloc((size_t)300);
    g3 = my_malloc((size_t)10);
    h4 = my_malloc((size_t)200);
    g4 = my_malloc((size_t)10);
    my_free(h1);
    my_free(h2);
    my_free(h3);
    my_free(h4);
    my_set_policy(BEST_FIT);
    x1 = my_malloc((size_t)150);
    x2 = my_malloc((size_t)151);
    x3 = my_malloc((size_t)250);
    x4 = my_malloc((size_t)10);
    x5 = my_malloc((size_t)280);
    my_heap_dump();
    return 0;
}
