/* realloc(p,0) checks the guard first, then frees (k=0) or delays (k=1) */
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
    a[32] = 0x00;
    r = my_realloc(a, (size_t)0ull);
    printf("r=%s\n", r ? "ptr" : "NULL");
    my_set_delay(1);
    b[37] = 0x00;
    r = my_realloc(b, (size_t)0ull);
    printf("r=%s\n", r ? "ptr" : "NULL");
    my_heap_dump();
    return 0;
}
