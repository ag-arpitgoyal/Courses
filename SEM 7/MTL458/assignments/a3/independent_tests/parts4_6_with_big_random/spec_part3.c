/* task.md Part 3 example (realloc grow in place, shrink in place) */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *p, *q, *r;

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
    p = my_malloc((size_t)100ull);
    q = my_malloc((size_t)200ull);
    r = my_malloc((size_t)10ull);
    my_free(q);
    p = my_realloc(p, (size_t)250ull);
    my_heap_dump();
    p = my_realloc(p, (size_t)20ull);
    my_heap_dump();
    return 0;
}
