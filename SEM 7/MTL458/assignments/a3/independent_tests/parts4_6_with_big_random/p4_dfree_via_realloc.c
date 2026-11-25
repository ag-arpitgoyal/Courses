/* my_realloc on freed / merged-away pointers reports errors and returns NULL; n==0 on freed ptr too */
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
    a = my_malloc((size_t)10ull);
    b = my_malloc((size_t)10ull);
    my_free(a);
    r = my_realloc(a, (size_t)50ull);
    printf("r=%s\n", r ? "ptr" : "NULL");
    my_free(b);
    r = my_realloc(b, (size_t)10ull);
    printf("r=%s\n", r ? "ptr" : "NULL");
    r = my_realloc(a, (size_t)0ull);
    printf("r=%s\n", r ? "ptr" : "NULL");
    my_heap_dump();
    return 0;
}
