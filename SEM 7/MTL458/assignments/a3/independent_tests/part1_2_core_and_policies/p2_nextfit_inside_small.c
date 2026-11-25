/* p2_nextfit_inside_small: next fit: position inside a coalesced free block that is too small -> next fitting block after it (not wrap); then inside and big enough */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *d, *e, *c2, *f, *g, *h;

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
    a = my_malloc((size_t)1000);
    b = my_malloc((size_t)10);
    c = my_malloc((size_t)10);
    d = my_malloc((size_t)10);
    e = my_malloc((size_t)10);
    my_free(a);
    my_free(c);
    my_set_policy(BEST_FIT);
    c2 = my_malloc((size_t)20);
    my_free(c2);
    my_free(d);
    my_set_policy(NEXT_FIT);
    f = my_malloc((size_t)300);
    my_heap_dump();
    my_set_policy(BEST_FIT);
    g = my_malloc((size_t)30);
    my_free(g);
    my_set_policy(NEXT_FIT);
    h = my_malloc((size_t)200);
    my_heap_dump();
    return 0;
}
