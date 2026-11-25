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
    my_heap_dump();
    unsigned char *a = my_calloc(3, 40);
    unsigned char *b = my_calloc(1, 1);
    unsigned char *c = my_calloc(10, 10);
    printf("a %d zero %d\n", a != NULL, a && allbytes(a, 120, 0));
    printf("b %d zero %d\n", b != NULL, b && allbytes(b, 1, 0));
    printf("c %d zero %d\n", c != NULL, c && allbytes(c, 100, 0));
    my_heap_dump();
    my_leak_report();
    a[127] = 0x11; /* last guard byte of a (n = 120) */
    my_free(a);
    my_free(b);
    my_heap_dump();
    my_leak_report();
    return 0;
}
