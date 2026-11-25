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
    my_set_delay(2);
    unsigned char *a = my_malloc(10);
    unsigned char *b = my_malloc(10);
    unsigned char *c = my_malloc(10);
    unsigned char *d = my_malloc(10);
    unsigned char *e = my_malloc(10);
    memset(a, 1, 10);
    memset(b, 2, 10);
    memset(c, 3, 10);
    memset(d, 4, 10);
    printf("%d\n", my_realloc(a, 0) == NULL);
    printf("%d\n", my_realloc(b, 0) == NULL);
    my_heap_dump();
    printf("a poisoned %d\n", allbytes(a, 10, 0xDE));
    b[0] = 'Q';  /* write after free, b is still delayed */
    c[12] = 0;   /* guard byte of c */
    printf("%d\n", my_realloc(c, 0) == NULL);
    printf("%d\n", my_realloc(d, 0) == NULL);
    my_heap_dump();
    printf("d poisoned %d\n", allbytes(d, 10, 0xDE));
    printf("%d\n", my_realloc(c, 20) == NULL);
    printf("%d\n", my_realloc(b, 20) == NULL);
    printf("%d\n", my_realloc(a, 0) == NULL);
    my_set_delay(1);
    my_heap_dump();
    my_set_delay(0);
    printf("%d\n", my_realloc(e, 0) == NULL);
    my_heap_dump();
    return 0;
}
