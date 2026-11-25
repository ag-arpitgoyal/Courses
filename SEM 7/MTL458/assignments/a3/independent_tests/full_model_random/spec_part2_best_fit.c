#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    void *a  = my_malloc(248);
    void *x1 = my_malloc(10);
    void *b  = my_malloc(120);
    void *x2 = my_malloc(10);
    void *c  = my_malloc(360);
    void *x3 = my_malloc(10);
    (void)x1; (void)x2; (void)x3;
    my_free(b);
    void *y  = my_malloc(120);
    my_free(y);
    my_free(a);
    my_free(c);
    my_set_policy(BEST_FIT);
    void *z = my_malloc(100);
    (void)z;
    my_heap_dump();
    return 0;
}
