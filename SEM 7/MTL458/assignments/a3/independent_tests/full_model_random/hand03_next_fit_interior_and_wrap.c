#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    void *a = my_malloc(10);
    void *b = my_malloc(10);
    void *c = my_malloc(10);
    void *d = my_malloc(10);
    (void)c; (void)d;
    my_free(a);
    my_free(b);
    void *x = my_malloc(10);
    my_free(x);
    my_set_policy(NEXT_FIT);
    void *y = my_malloc(100);
    my_heap_dump();
    my_free(y);
    void *w = my_malloc(64900);
    void *u = my_malloc(10);
    (void)w; (void)u;
    my_heap_dump();
    return 0;
}
