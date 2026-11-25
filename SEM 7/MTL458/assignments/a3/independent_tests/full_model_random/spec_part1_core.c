#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    void *a = my_malloc(10);
    void *b = my_malloc(100);
    void *c = my_malloc(10);
    (void)c;
    my_heap_dump();
    my_free(b);
    my_free(a);
    void *d = my_malloc(100000);
    (void)d;
    my_heap_dump();
    return 0;
}
