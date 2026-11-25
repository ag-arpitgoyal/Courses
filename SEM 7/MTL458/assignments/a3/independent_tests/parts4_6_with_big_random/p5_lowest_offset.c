/* buddy: among equal-size free blocks the lowest offset is used */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *m0, *m1, *m2, *m3, *m4, *m5, *m6, *m7, *x, *y, *z;

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
    m0 = my_malloc((size_t)40ull);
    m1 = my_malloc((size_t)40ull);
    m2 = my_malloc((size_t)40ull);
    m3 = my_malloc((size_t)40ull);
    m4 = my_malloc((size_t)40ull);
    m5 = my_malloc((size_t)40ull);
    m6 = my_malloc((size_t)40ull);
    m7 = my_malloc((size_t)40ull);
    my_free(m2);
    my_free(m5);
    my_heap_dump();
    x = my_malloc((size_t)40ull);
    y = my_malloc((size_t)40ull);
    z = my_malloc((size_t)40ull);
    my_heap_dump();
    return 0;
}
