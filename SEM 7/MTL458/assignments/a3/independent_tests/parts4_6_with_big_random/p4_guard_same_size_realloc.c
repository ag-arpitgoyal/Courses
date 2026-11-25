/* realloc to the same n resets the guard (block is resized in place) */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *x;

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
    a = my_malloc((size_t)40ull);
    x = my_malloc((size_t)10ull);
    a[41] = 0x00;
    a = my_realloc(a, (size_t)40ull);
    my_free(a);
    my_heap_dump();
    return 0;
}
