#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

int main(void) {
    unsigned char *a = my_malloc(10);
    unsigned char *b = my_malloc(100);
    unsigned char *c = my_malloc(10);
    unsigned char *d = my_malloc(10);
    printf("%d\n", my_realloc(b, 0) == NULL);
    my_heap_dump();
    printf("%d\n", my_realloc(a, 0) == NULL);
    c[10] = 'x'; /* first guard byte */
    printf("%d\n", my_realloc(c, 0) == NULL);
    my_heap_dump();
    my_free(c);
    my_free(a);
    printf("%d\n", my_realloc(d, 0) == NULL);
    my_heap_dump();
    my_leak_report();
    return 0;
}
