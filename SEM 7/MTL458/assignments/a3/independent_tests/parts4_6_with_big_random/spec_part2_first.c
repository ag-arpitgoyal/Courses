/* task.md Part 2 table: first fit row */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *x1, *b, *x2, *c, *x3, *y, *z;

static int allb(const unsigned char *p, int v, size_t c)
{
    for (size_t i = 0; i < c; i++)
        if (p[i] != (unsigned char)v)
            return 0;
    return 1;
}

int main(void)
{
    unsigned char STACK_BUF[64];
    memset(STACK_BUF, 0, sizeof STACK_BUF);
    (void)STATIC_BUF; (void)allb;
    a = my_malloc((size_t)248ull);
    x1 = my_malloc((size_t)10ull);
    b = my_malloc((size_t)120ull);
    x2 = my_malloc((size_t)10ull);
    c = my_malloc((size_t)360ull);
    x3 = my_malloc((size_t)10ull);
    my_free(b);
    y = my_malloc((size_t)120ull);
    my_free(y);
    my_free(a);
    my_free(c);
    my_heap_dump();
    my_set_policy(FIRST_FIT);
    z = my_malloc((size_t)100ull);
    my_heap_dump();
    return 0;
}
