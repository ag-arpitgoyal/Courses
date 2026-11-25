/* after realloc (same-block, shrink+split, grow in place) the guard follows the new n; old guard bytes become data */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *p, *q, *r, *p0, *s;

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
    p = my_malloc((size_t)10ull);
    q = my_malloc((size_t)10ull);
    r = my_malloc((size_t)10ull);
    p0 = p;
    p = my_realloc(p, (size_t)80ull);
    printf("p==p0 %s\n", p == p0 ? "yes" : "no");
    p[10] = 0x41;
    p[17] = 0x41;
    my_free(p);
    q = my_realloc(q, (size_t)70ull);
    q[77] = 0x00;
    my_free(q);
    s = my_malloc((size_t)200ull);
    s = my_realloc(s, (size_t)5ull);
    s[5] = 0x00;
    my_heap_dump();
    my_free(s);
    r = my_realloc(r, (size_t)300ull);
    r[303] = 0x00;
    my_heap_dump();
    my_free(r);
    my_heap_dump();
    return 0;
}
