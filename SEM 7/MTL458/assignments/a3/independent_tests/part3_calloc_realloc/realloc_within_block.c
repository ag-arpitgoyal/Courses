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
static int allbytes(const void *p, size_t n, unsigned char v) {
    const unsigned char *c = p;
    for (size_t i = 0; i < n; i++)
        if (c[i] != v) return 0;
    return 1;
}
static void hex(const char *tag, const unsigned char *p, size_t n) {
    printf("%s:", tag);
    for (size_t i = 0; i < n; i++) printf(" %02x", p[i]);
    printf("\n");
}

int main(void) {
    unsigned char *a = my_malloc(10);
    unsigned char *b = my_malloc(10);
    fill(a, 10, 1);
    /* n grows but block_size(80) = 128 still fits */
    printf("same %d\n", my_realloc(a, 80) == a);
    hex("a", a, 10);
    memset(a + 10, 0x55, 70); /* old guard bytes are now user data */
    printf("same %d\n", my_realloc(a, 88) == a);
    memset(a + 80, 0x66, 8);
    my_heap_dump();
    a[95] = 0; /* last guard byte for n = 88 */
    unsigned char *q = my_realloc(a, 89); /* block_size 144: must move */
    printf("moved %d\n", q != a);
    printf("q data %d %d %d\n", same(q, 10, 1), allbytes(q + 10, 70, 0x55), allbytes(q + 80, 8, 0x66));
    hex("q", q, 12);

    printf("same %d\n", my_realloc(b, 50) == b);
    memset(b + 10, 0x77, 40);
    my_free(b); /* no overflow: guard moved to b[50..57] */
    my_heap_dump();

    unsigned char *d = my_malloc(20);
    printf("same %d\n", my_realloc(d, 60) == d);
    d[60] = 1;
    my_free(d);

    /* a block with spare room because it was not split */
    unsigned char *x = my_malloc(200);
    fill(x, 200, 9);
    printf("same %d\n", my_realloc(x, 210) == x);
    printf("same %d\n", my_realloc(x, 216) == x);
    my_heap_dump();
    unsigned char *y = my_realloc(x, 217);
    printf("moved %d data %d\n", y != x, same(y, 200, 9));
    my_heap_dump();
    my_leak_report();
    return 0;
}
