/* p1_block_sizes: block_size formula: min 128 (n=1,24,88), 89->144, 104/105, 120/121, 1000/1001, 1016/1017; blocks laid end to end */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *d, *e, *f, *g, *h, *i, *j, *k, *l;

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
    a = my_malloc((size_t)1);
    b = my_malloc((size_t)24);
    c = my_malloc((size_t)88);
    d = my_malloc((size_t)89);
    e = my_malloc((size_t)104);
    f = my_malloc((size_t)105);
    g = my_malloc((size_t)120);
    h = my_malloc((size_t)121);
    i = my_malloc((size_t)1000);
    j = my_malloc((size_t)1001);
    k = my_malloc((size_t)1016);
    l = my_malloc((size_t)1017);
    my_heap_dump();
    return 0;
}
