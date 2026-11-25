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

#define G ((size_t)1 << 30)

int main(void) {
    unsigned char *a = my_malloc(100);
    unsigned char *b = my_malloc(10);
    fill(a, 100, 5);
    fill(b, 10, 6);
    printf("%d\n", my_realloc(a, G + 1) == NULL);
    printf("%d\n", my_realloc(a, SIZE_MAX) == NULL);
    printf("%d\n", my_realloc(a, (size_t)1 << 40) == NULL);
    printf("a intact %d\n", same(a, 100, 5));
    a[100] = 0x00; /* guard byte */
    printf("%d\n", my_realloc(a, G + 1) == NULL);
    a[100] = 0xCA; /* restore the guard byte */
    printf("%d\n", my_realloc(a, SIZE_MAX) == NULL);
    printf("a intact %d\n", same(a, 100, 5));
    my_heap_dump();
    my_free(a);
    my_heap_dump();
    printf("%d\n", my_realloc(b, G + 1) == NULL);
    my_heap_dump();
    /* exactly 2^30: moves to a new region */
    unsigned char *c = my_realloc(b, G);
    printf("c %d data %d\n", c != NULL, c && same(c, 10, 6));
    my_heap_dump();
    c[G] = 0; /* guard byte after the new n */
    unsigned char *d = my_realloc(c, 5);
    printf("d same %d data %d\n", d == c, same(d, 5, 6));
    my_heap_dump();
    my_leak_report();
    return 0;
}
