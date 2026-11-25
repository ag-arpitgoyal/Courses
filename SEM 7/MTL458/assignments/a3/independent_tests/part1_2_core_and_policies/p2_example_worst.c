/* p2_example_worst: Part 2 example then WORST_FIT malloc(100) */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *x1, *b, *x2, *c, *x3, *y, *z;

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
    a = my_malloc((size_t)248);
    x1 = my_malloc((size_t)10);
    b = my_malloc((size_t)120);
    x2 = my_malloc((size_t)10);
    c = my_malloc((size_t)360);
    x3 = my_malloc((size_t)10);
    my_free(b);
    y = my_malloc((size_t)120);
    my_free(y);
    my_free(a);
    my_free(c);
    my_heap_dump();
    my_set_policy(WORST_FIT);
    z = my_malloc((size_t)100);
    my_heap_dump();
    return 0;
}
