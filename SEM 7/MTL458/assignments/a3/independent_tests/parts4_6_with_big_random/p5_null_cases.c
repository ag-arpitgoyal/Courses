/* buddy: NULL for n==0, n=1048561, no large-enough block (heap unchanged); n=1048560 takes the whole region */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *b, *c, *d, *e, *f, *g, *h;

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
    a = my_malloc((size_t)10ull);
    b = my_malloc((size_t)0ull);
    printf("b=%s\n", b ? "ptr" : "NULL");
    c = my_malloc((size_t)1048561ull);
    printf("c=%s\n", c ? "ptr" : "NULL");
    d = my_malloc((size_t)1048560ull);
    printf("d=%s\n", d ? "ptr" : "NULL");
    e = my_malloc((size_t)524272ull);
    printf("e=%s\n", e ? "ptr" : "NULL");
    f = my_malloc((size_t)262129ull);
    printf("f=%s\n", f ? "ptr" : "NULL");
    my_heap_dump();
    my_free(a);
    my_free(e);
    g = my_malloc((size_t)1048560ull);
    printf("g=%s\n", g ? "ptr" : "NULL");
    my_heap_dump();
    h = my_malloc((size_t)1ull);
    printf("h=%s\n", h ? "ptr" : "NULL");
    my_free(g);
    my_heap_dump();
    return 0;
}
