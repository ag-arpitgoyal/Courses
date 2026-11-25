#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static int allbytes(const void *p, size_t n, unsigned char v) {
    const unsigned char *c = p;
    for (size_t i = 0; i < n; i++)
        if (c[i] != v) return 0;
    return 1;
}

int main(void) {
    unsigned char *a = my_malloc(200);
    unsigned char *b = my_malloc(100);
    unsigned char *c = my_malloc(50);
    memset(a, 0xAB, 200);
    memset(b, 0xCD, 100);
    memset(c, 0xEF, 50);
    my_free(a);
    unsigned char *p = my_calloc(25, 8);
    printf("p reused %d zero %d\n", p == a, allbytes(p, 200, 0));
    memset(p, 0x99, 200);
    my_free(p);
    my_free(b);
    /* spans the old a block and the old b block (including b's header) */
    unsigned char *q = my_calloc(1, 300);
    printf("q at a %d zero %d\n", q == a, allbytes(q, 300, 0));
    memset(q, 0x77, 300);
    my_heap_dump();
    my_set_delay(1);
    my_free(c);
    my_free(q);
    my_heap_dump();
    my_set_delay(0);
    /* covers the old q and c blocks, which were filled with 0xDE while delayed */
    unsigned char *r = my_calloc(1, 600);
    printf("r at a %d zero %d\n", r == a, allbytes(r, 600, 0));
    my_heap_dump();
    my_leak_report();
    return 0;
}
