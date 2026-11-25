/* clarification 3: realloc(p, n>2^30) checks guard (message only if modified), returns NULL, block and data unchanged */
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
    a = my_malloc((size_t)30ull);
    b = my_malloc((size_t)30ull);
    memset(a, 0x33, 30);
    r = my_realloc(b, (size_t)1073741825ull);
    printf("r=%s\n", r ? "ptr" : "NULL");
    a[35] = 0x00;
    r = my_realloc(a, (size_t)1073741825ull);
    printf("r=%s\n", r ? "ptr" : "NULL");
    printf("check a %s\n", allb(a, 0x33, 30) ? "ok" : "BAD");
    my_heap_dump();
    return 0;
}
