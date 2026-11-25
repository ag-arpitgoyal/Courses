#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    unsigned char *a = my_calloc(1, 65400);
    for (int i = 0; i < 65400; i++) {
        if (a[i] != 0) { printf("NOT ZERO a\n"); break; }
        a[i] = 0xAA;
    }
    unsigned char *b = my_calloc(200, 500);
    for (int i = 0; i < 100000; i++) {
        if (b[i] != 0) { printf("NOT ZERO b\n"); break; }
    }
    my_free(a);
    unsigned char *c = my_calloc(10, 10);
    for (int i = 0; i < 100; i++) {
        if (c[i] != 0) { printf("NOT ZERO c\n"); break; }
    }
    my_heap_dump();
    return 0;
}
