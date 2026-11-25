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

int main(void) {
    my_set_delay(1);
    unsigned char *a = my_malloc(10);
    unsigned char *b = my_malloc(10);
    unsigned char *c = my_malloc(10);
    unsigned char *d = my_malloc(10);
    fill(a, 10, 1);
    fill(b, 10, 2);
    fill(c, 10, 3);
    fill(d, 10, 4);
    my_free(b);
    /* next block is delayed: cannot grow into it, must move */
    unsigned char *q = my_realloc(a, 100);
    printf("moved %d data %d\n", q != a, same(q, 10, 1));
    printf("old poisoned %d\n", allbytes(a, 10, 0xDE));
    my_heap_dump();
    fill(q, 100, 1);
    a[2] = 'Z'; /* write after free: old block is still in the queue */
    printf("null %d\n", my_realloc(c, 0) == NULL);
    my_heap_dump();

    /* the new block is placed before the queue drops c (which would make 0:32 big enough) */
    unsigned char *d2 = my_realloc(d, 300);
    printf("moved %d data %d\n", d2 != d, same(d2, 10, 4));
    fill(d2, 300, 4);
    my_heap_dump();

    /* overflow on a moved block is reported once */
    q[100] = 0;
    unsigned char *r = my_realloc(q, 1000);
    printf("moved %d data %d\n", r != q, same(r, 100, 1));
    fill(r, 1000, 1);
    printf("null %d\n", my_realloc(q, 50) == NULL);
    /* grow in place still works while delay > 0 */
    printf("same %d\n", my_realloc(r, 2000) == r);
    my_heap_dump();
    my_set_delay(0);
    my_heap_dump();
    printf("data %d %d\n", same(d2, 300, 4), same(r, 1000, 1));
    my_leak_report();
    return 0;
}
