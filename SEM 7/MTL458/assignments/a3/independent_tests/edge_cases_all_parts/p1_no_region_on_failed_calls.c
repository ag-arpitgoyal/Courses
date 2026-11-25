#include "myalloc.h"
#include <stdio.h>

int main(void) {
    my_heap_dump();
    void *p1 = my_malloc(0);
    void *p2 = my_malloc((1ULL << 30) + 1);
    void *p3 = my_calloc(0, 100);
    void *p4 = my_calloc(100, 0);
    void *p5 = my_calloc(1ULL << 16, (1ULL << 14) + 1);
    void *p6 = my_calloc(1ULL << 33, 1ULL << 32);
    void *p7 = my_realloc(NULL, 0);
    void *p8 = my_realloc(NULL, (1ULL << 30) + 1);
    my_free(NULL);
    if (p1 || p2 || p3 || p4 || p5 || p6 || p7 || p8) {
        printf("unexpected non-null\n");
    }
    my_heap_dump();
    my_leak_report();
    return 0;
}
