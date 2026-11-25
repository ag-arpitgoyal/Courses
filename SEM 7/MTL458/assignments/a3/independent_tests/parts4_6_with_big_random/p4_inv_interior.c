/* pointers into the middle of blocks / misaligned / into next header: invalid pointer, nothing changes */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *b, *r;

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
    a = my_malloc((size_t)100ull);
    b = my_malloc((size_t)10ull);
    my_free((a + 1));
    my_free((a + 3));
    my_free((a + 8));
    my_free((a + 16));
    my_free((a + 32));
    my_free((a + 99));
    my_free((a + 100));
    my_free((a + 128));
    my_free((b - 16));
    my_free((b - 8));
    r = my_realloc((a + 16), (size_t)200ull);
    printf("r=%s\n", r ? "ptr" : "NULL");
    my_heap_dump();
    my_free(a);
    my_free(b);
    my_heap_dump();
    return 0;
}
