#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    void *p = my_malloc(100);
    void *q = my_malloc(200);
    void *r = my_malloc(10);
    (void)r;
    my_free(q);
    p = my_realloc(p, 250);
    my_heap_dump();
    p = my_realloc(p, 20);
    (void)p;
    my_heap_dump();
    return 0;
}
