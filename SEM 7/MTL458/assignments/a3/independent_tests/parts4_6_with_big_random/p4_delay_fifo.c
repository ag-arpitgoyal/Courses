/* FIFO eviction order, delayed neighbours block coalescing, lowering k evicts from the front, k=0 drains */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *b, *c, *d, *e;

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
    my_set_delay(3);
    a = my_malloc((size_t)10ull);
    b = my_malloc((size_t)10ull);
    c = my_malloc((size_t)10ull);
    d = my_malloc((size_t)10ull);
    e = my_malloc((size_t)10ull);
    my_free(b);
    my_free(d);
    my_free(a);
    my_heap_dump();
    my_free(e);
    my_free(c);
    my_heap_dump();
    my_set_delay(1);
    my_heap_dump();
    my_set_delay(0);
    my_heap_dump();
    return 0;
}
