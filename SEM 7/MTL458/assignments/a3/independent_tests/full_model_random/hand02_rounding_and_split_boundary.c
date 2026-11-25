#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    void *a = my_malloc(216);
    void *b = my_malloc(10);
    (void)b;
    my_free(a);
    void *c = my_malloc(104);
    my_heap_dump();
    my_free(c);
    void *d = my_malloc(88);
    (void)d;
    my_heap_dump();
    return 0;
}
