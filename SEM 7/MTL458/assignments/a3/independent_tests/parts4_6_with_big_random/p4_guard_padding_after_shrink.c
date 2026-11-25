/* bytes after the guard (old data turned into padding by an in-place shrink) are not checked; the new guard is */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *p, *q, *x;

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
    p = my_malloc((size_t)96ull);
    q = my_malloc((size_t)96ull);
    x = my_malloc((size_t)10ull);
    memset(p, 0x5A, 96);
    memset(q, 0x5A, 96);
    p = my_realloc(p, (size_t)72ull);
    q = my_realloc(q, (size_t)72ull);
    printf("check p %s\n", allb(p, 0x5A, 72) ? "ok" : "BAD");
    printf("check q %s\n", allb(q, 0x5A, 72) ? "ok" : "BAD");
    q[72] = 0x00;
    my_free(p);
    my_free(q);
    my_heap_dump();
    return 0;
}
