/* realloc shrink-in-place leftover does not coalesce with a delayed next block */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *p, *q;

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
    p = my_malloc((size_t)500ull);
    q = my_malloc((size_t)10ull);
    my_free(q);
    p = my_realloc(p, (size_t)10ull);
    my_heap_dump();
    return 0;
}
