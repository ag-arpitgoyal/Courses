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
    unsigned char *a = my_malloc(100);
    unsigned char *b = my_malloc(100);
    unsigned char *c = my_malloc(100);
    unsigned char *d = my_malloc(100);
    unsigned char *e = my_malloc(100);
    unsigned char *f = my_malloc(100);
    unsigned char *g = my_malloc(100);
    unsigned char *h = my_malloc(100);
    unsigned char *i = my_malloc(100);
    fill(a, 100, 1);
    fill(c, 100, 3);
    fill(e, 100, 5);
    fill(g, 100, 7);
    fill(i, 100, 9);

    my_free(b);
    printf("exact %d\n", my_realloc(a, 248) == a);           /* 144 + 144 = 288 = B */
    my_free(d);
    printf("split128 %d\n", my_realloc(c, 120) == c);        /* leftover 128: split */
    my_free(f);
    printf("absorb16 %d\n", my_realloc(e, 232) == e);        /* leftover 16 */
    my_free(h);
    printf("absorb112 %d\n", my_realloc(g, 136) == g);       /* leftover 112 */
    printf("tail %d\n", my_realloc(i, 1000) == i);           /* into the region's tail */
    printf("data %d %d %d %d %d\n", same(a, 100, 1), same(c, 100, 3), same(e, 100, 5),
           same(g, 100, 7), same(i, 100, 9));
    my_heap_dump();
    fill(i, 1000, 9);

    /* next block free but too small: move */
    unsigned char *c2 = my_realloc(c, 300);
    printf("c moved %d data %d\n", c2 != c, same(c2, 100, 3));
    fill(c2, 300, 3);
    /* free blocks on both sides; prev+old+next would fit but must not be used */
    my_free(g);
    unsigned char *e2 = my_realloc(e, 600);
    printf("e moved %d data %d\n", e2 != e, same(e2, 100, 5));
    fill(e2, 600, 5);
    my_heap_dump();
    /* free block before is large enough, but the block before is never used */
    unsigned char *i2 = my_realloc(i, 1500);
    printf("i moved %d data %d\n", i2 != i, same(i2, 1000, 9));
    fill(i2, 1500, 9);
    my_heap_dump();

    /* next block delayed: move */
    my_set_delay(1);
    my_free(e2);
    unsigned char *c3 = my_realloc(c2, 400);
    printf("c moved %d data %d\n", c3 != c2, same(c3, 300, 3));
    my_heap_dump();
    my_set_delay(0);

    /* grow into the region's tail, then exactly to the end of the region */
    printf("same %d\n", my_realloc(i2, 3000) == i2);
    fill(i2, 3000, 9);
    printf("same %d\n", my_realloc(i2, 62280) == i2);
    fill(i2, 62280, 9);
    my_heap_dump();
    /* last block of the region: nothing to grow into */
    unsigned char *i3 = my_realloc(i2, 62281);
    printf("i moved %d data %d\n", i3 != i2, same(i3, 62280, 9));
    my_heap_dump();
    my_leak_report();
    return 0;
}
