/* guard_realloc_grow: after grow in place the old guard spot is ordinary data and the guard follows the new n */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static void dumpbytes(const char *p, int len)
{
    for (int i = 0; i < len; i++)
        printf("%02x%c", (unsigned char)p[i], i + 1 < len ? ' ' : '\n');
}

int main(void)
{
    char *p, *p2, *q, *r, *x, *x2, *y, *y2;
    p = my_malloc(10);
    q = my_malloc(10);
    r = my_malloc(10);
    my_free(q);
    p2 = my_realloc(p, 100);
    printf("p2==p %d\n", p2 == p);
    memset(p2, 0x33, 100);
    my_free(p2);
    x = my_malloc(10);
    x2 = my_realloc(x, 100);
    ((unsigned char *)x2)[107] = 0xEE;
    my_free(x2);
    y = my_malloc(10);
    ((unsigned char *)y)[10] = 0x00;
    y2 = my_realloc(y, 100);
    dumpbytes(y2 + 100, 8);
    my_free(y2);
    my_heap_dump();
    (void)p; (void)p2; (void)q; (void)r; (void)x; (void)x2; (void)y; (void)y2;
    return 0;
}
