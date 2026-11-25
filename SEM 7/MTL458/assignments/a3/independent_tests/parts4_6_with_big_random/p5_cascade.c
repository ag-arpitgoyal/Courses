/* buddy: freeing merges only same-size buddies, then cascades all the way up */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *b, *c;

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
    a = my_malloc((size_t)1ull);
    b = my_malloc((size_t)1ull);
    c = my_malloc((size_t)40ull);
    my_free(a);
    my_free(c);
    my_heap_dump();
    my_free(b);
    my_heap_dump();
    return 0;
}
