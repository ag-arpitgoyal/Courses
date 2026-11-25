/* delayed blocks are never reused by first/best/worst fit */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *x, *b, *c, *d;

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
    my_set_delay(2);
    a = my_malloc((size_t)10ull);
    x = my_malloc((size_t)10ull);
    my_free(a);
    b = my_malloc((size_t)10ull);
    my_set_policy(BEST_FIT);
    c = my_malloc((size_t)10ull);
    my_heap_dump();
    my_set_policy(WORST_FIT);
    my_free(x);
    d = my_malloc((size_t)10ull);
    my_heap_dump();
    return 0;
}
