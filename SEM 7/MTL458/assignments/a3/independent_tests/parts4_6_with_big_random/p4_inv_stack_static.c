/* my_free/my_realloc of stack and static pointers: invalid pointer, heap unchanged, realloc returns NULL */
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
    a = my_malloc((size_t)10ull);
    my_free((STACK_BUF + 16));
    my_free((STATIC_BUF + 64));
    r = my_realloc((STACK_BUF + 16), (size_t)50ull);
    printf("r=%s\n", r ? "ptr" : "NULL");
    r = my_realloc((STATIC_BUF+16), (size_t)50ull);
    printf("r=%s\n", r ? "ptr" : "NULL");
    my_heap_dump();
    my_free(a);
    my_heap_dump();
    return 0;
}
