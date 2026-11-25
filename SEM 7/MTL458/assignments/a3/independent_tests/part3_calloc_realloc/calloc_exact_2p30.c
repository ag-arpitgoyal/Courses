#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

#define G ((size_t)1 << 30)

static int sampled_zero(const unsigned char *p) {
    for (size_t i = 0; i < G; i += 4093)
        if (p[i] != 0) return 0;
    if (p[0] || p[1] || p[4095] || p[4096] || p[G / 2] || p[G - 2] || p[G - 1]) return 0;
    return 1;
}

int main(void) {
    /* exactly 2^30 bytes as the very first request: region 0 sized by the Growth rule */
    unsigned char *p = my_calloc((size_t)1 << 15, (size_t)1 << 15);
    printf("p %d\n", p != NULL);
    if (!p) return 0;
    printf("zero %d\n", sampled_zero(p));
    my_heap_dump();
    printf("over %d\n", my_calloc(1, G + 1) == NULL);
    printf("realloc over %d\n", my_realloc(p, G + 1) == NULL);
    printf("realloc same %d\n", my_realloc(p, G) == p);
    p[0] = 1;
    p[12345] = 2;
    p[G / 2] = 3;
    p[G - 1] = 4;
    my_free(p);
    my_heap_dump();
    /* reuses the same, now dirty, block */
    unsigned char *q = my_calloc(G, 1);
    printf("q same %d\n", q == p);
    printf("zero %d\n", q && sampled_zero(q) && q[12345] == 0);
    my_heap_dump();
    my_free(q);
    my_leak_report();
    return 0;
}
