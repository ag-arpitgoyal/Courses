/* clarification 7: data address of a never-returned split leftover (malloc split, realloc split, growth leftover) = double free */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *r, *p, *d;

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
    a = my_malloc((size_t)10ull);
    my_free((a + 128));
    r = my_realloc((a + 128), (size_t)40ull);
    printf("r=%s\n", r ? "ptr" : "NULL");
    p = my_malloc((size_t)1000ull);
    p = my_realloc(p, (size_t)10ull);
    my_free((p + 128));
    d = my_malloc((size_t)100000ull);
    my_free((d + 100048));
    my_heap_dump();
    return 0;
}
