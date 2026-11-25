/* p2_example_next: Part 2 example then NEXT_FIT malloc(100) x2; position after an unsplit block is its real end, and a freed block ending exactly at the position is not chosen */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *x1, *b, *x2, *c, *x3, *y, *z, *z2, *z3;

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
    my_set_policy(NEXT_FIT);
    z = my_malloc((size_t)100);
    my_heap_dump();
    z2 = my_malloc((size_t)100);
    my_heap_dump();
    my_free(z2);
    z3 = my_malloc((size_t)100);
    my_heap_dump();
    return 0;
}
