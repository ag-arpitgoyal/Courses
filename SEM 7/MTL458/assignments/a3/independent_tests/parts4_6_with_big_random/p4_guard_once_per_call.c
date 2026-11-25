/* all 8 guard bytes changed: one message per call (free, realloc Move, realloc Move with delay) */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *x, *y;

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
    a = my_malloc((size_t)20ull);
    x = my_malloc((size_t)20ull);
    a[20] = 0x00;
    a[21] = 0x00;
    a[22] = 0x00;
    a[23] = 0x00;
    a[24] = 0x00;
    a[25] = 0x00;
    a[26] = 0x00;
    a[27] = 0x00;
    a = my_realloc(a, (size_t)500ull);
    my_heap_dump();
    x[20] = 0x11;
    x[21] = 0x11;
    x[22] = 0x11;
    x[23] = 0x11;
    x[24] = 0x11;
    x[25] = 0x11;
    x[26] = 0x11;
    x[27] = 0x11;
    my_free(x);
    my_heap_dump();
    my_set_delay(1);
    y = my_malloc((size_t)20ull);
    y[20] = 0x00;
    y[21] = 0x00;
    y[22] = 0x00;
    y[23] = 0x00;
    y[24] = 0x00;
    y[25] = 0x00;
    y[26] = 0x00;
    y[27] = 0x00;
    y = my_realloc(y, (size_t)300ull);
    my_heap_dump();
    my_set_delay(0);
    my_heap_dump();
    return 0;
}
