/* buddy: block size = max(32, pow2 >= n+16) at the boundaries 16/17, 48/49, 112/113, 1008/1009 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *b, *c, *d, *e, *f, *g, *h;

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
    a = my_malloc((size_t)16ull);
    b = my_malloc((size_t)17ull);
    c = my_malloc((size_t)48ull);
    d = my_malloc((size_t)49ull);
    e = my_malloc((size_t)112ull);
    f = my_malloc((size_t)113ull);
    my_heap_dump();
    g = my_malloc((size_t)1009ull);
    h = my_malloc((size_t)1008ull);
    my_heap_dump();
    return 0;
}
