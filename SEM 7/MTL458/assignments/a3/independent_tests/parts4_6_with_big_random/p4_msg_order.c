/* overflow of the block being freed is printed before write-after-free of the evicted block (my_free and realloc Move) */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *b, *c, *x;

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
    my_set_delay(1);
    a = my_malloc((size_t)10ull);
    b = my_malloc((size_t)10ull);
    my_free(a);
    a[0] = 0x00;
    b[10] = 0x00;
    my_free(b);
    my_heap_dump();
    c = my_malloc((size_t)10ull);
    x = my_malloc((size_t)10ull);
    b[5] = 0x00;
    c[13] = 0x00;
    c = my_realloc(c, (size_t)200ull);
    my_heap_dump();
    return 0;
}
