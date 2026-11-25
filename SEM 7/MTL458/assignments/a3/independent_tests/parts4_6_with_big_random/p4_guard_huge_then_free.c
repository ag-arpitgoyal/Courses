/* after realloc(p, n>2^30) leaves the block unchanged, its modified guard is reported again by my_free */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *r;

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
    a = my_malloc((size_t)30ull);
    a[35] = 0x00;
    r = my_realloc(a, (size_t)1073741825ull);
    my_free(a);
    my_heap_dump();
    return 0;
}
