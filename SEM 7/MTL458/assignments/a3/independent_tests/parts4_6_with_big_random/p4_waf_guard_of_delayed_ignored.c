/* eviction checks only the n bytes: writing a delayed block's guard bytes is not reported */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *b;

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
    a[10] = 0x00;
    a[17] = 0x00;
    my_free(b);
    my_heap_dump();
    return 0;
}
