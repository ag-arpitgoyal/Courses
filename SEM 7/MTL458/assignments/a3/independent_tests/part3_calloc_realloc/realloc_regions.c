#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static void fill(void *p, size_t n, unsigned seed) {
    unsigned char *c = p;
    for (size_t i = 0; i < n; i++) c[i] = (unsigned char)(seed + i * 7 + (i >> 8));
}
static int same(const void *p, size_t n, unsigned seed) {
    const unsigned char *c = p;
    for (size_t i = 0; i < n; i++)
        if (c[i] != (unsigned char)(seed + i * 7 + (i >> 8))) return 0;
    return 1;
}

int main(void) {
    unsigned char *a = my_malloc(30000);
    unsigned char *b = my_malloc(30000);
    fill(a, 30000, 1);
    fill(b, 30000, 2);
    my_heap_dump();
    /* nothing fits: new region 1 */
    unsigned char *a2 = my_realloc(a, 40000);
    printf("moved %d data %d\n", a2 != a, same(a2, 30000, 1));
    fill(a2, 40000, 1);
    /* grow in place with split, then exactly up to the end of region 0 */
    printf("same %d\n", my_realloc(b, 30010) == b);
    fill(b, 30010, 2);
    printf("same %d\n", my_realloc(b, 35416) == b);
    fill(b, 35416, 2);
    my_heap_dump();
    /* last block of region 0 grows: no next block, move to new region 2 */
    unsigned char *b2 = my_realloc(b, 35417);
    printf("moved %d data %d\n", b2 != b, same(b2, 35416, 2));
    fill(b2, 35417, 2);
    my_heap_dump();
    /* 100000 bytes: region 3 of 131072 */
    unsigned char *c = my_realloc(a2, 100000);
    printf("moved %d data %d\n", c != a2, same(c, 40000, 1));
    fill(c, 100000, 1);
    printf("same %d\n", my_realloc(c, 65000) == c);
    unsigned char *d = my_malloc(65464);
    fill(d, 65464, 4);
    printf("same %d\n", my_realloc(d, 100) == d);
    my_heap_dump();
    unsigned char *f = my_malloc(10);
    /* moves from region 2 to a later region (3) */
    unsigned char *b3 = my_realloc(b2, 65465);
    printf("moved %d data %d\n", b3 != b2, same(b3, 35417, 2));
    /* moves from region 3 to an earlier region (1), leftover 64 absorbed */
    unsigned char *c2 = my_realloc(c, 65400);
    printf("moved %d data %d\n", c2 != c, same(c2, 65000, 1));
    my_heap_dump();
    printf("data %d\n", same(d, 100, 4));
    my_leak_report();
    (void)f;
    return 0;
}
