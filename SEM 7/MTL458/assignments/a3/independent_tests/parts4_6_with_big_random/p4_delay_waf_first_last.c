/* write-after-free at first/last/only byte detected once per block; writing 0xDE is undetectable */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *b, *c, *d;

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
    my_set_delay(4);
    a = my_malloc((size_t)10ull);
    b = my_malloc((size_t)50ull);
    c = my_malloc((size_t)1ull);
    d = my_malloc((size_t)100ull);
    my_free(a);
    my_free(b);
    my_free(c);
    my_free(d);
    a[0] = 0x00;
    a[5] = 0x00;
    a[9] = 0x00;
    b[49] = 0x41;
    c[0] = 0x41;
    d[50] = 0xDE;
    d[0] = 0xDE;
    my_heap_dump();
    my_set_delay(0);
    my_heap_dump();
    return 0;
}
