/* delay via realloc(p,0) and realloc Move (data copied before 0xDE fill); double frees of both; WAF via old pointer */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *x, *b, *y, *r, *ob;

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
    my_set_delay(2);
    a = my_malloc((size_t)10ull);
    x = my_malloc((size_t)10ull);
    b = my_malloc((size_t)10ull);
    y = my_malloc((size_t)10ull);
    memset(b, 0x5A, 10);
    r = my_realloc(a, (size_t)0ull);
    printf("r=%s\n", r ? "ptr" : "NULL");
    ob = b;
    b = my_realloc(b, (size_t)500ull);
    printf("check b %s\n", allb(b, 0x5A, 10) ? "ok" : "BAD");
    my_heap_dump();
    my_free(a);
    my_free(ob);
    r = my_realloc(ob, (size_t)20ull);
    ob[3] = 0x00;
    my_free(x);
    my_free(y);
    my_heap_dump();
    my_set_delay(0);
    my_heap_dump();
    my_leak_report();
    return 0;
}
