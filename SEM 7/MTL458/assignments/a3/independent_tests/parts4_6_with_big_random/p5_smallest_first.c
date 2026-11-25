/* buddy: smallest fitting block preferred over a lower-offset larger one; split keeps lower half */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *b, *c, *d, *e, *f, *g, *h, *i;

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
    my_set_policy(BUDDY);
    a = my_malloc((size_t)1ull);
    b = my_malloc((size_t)1ull);
    c = my_malloc((size_t)1ull);
    d = my_malloc((size_t)1ull);
    my_free(a);
    my_free(b);
    my_free(d);
    my_heap_dump();
    e = my_malloc((size_t)1ull);
    f = my_malloc((size_t)1ull);
    g = my_malloc((size_t)100ull);
    h = my_malloc((size_t)100ull);
    i = my_malloc((size_t)17ull);
    my_heap_dump();
    return 0;
}
