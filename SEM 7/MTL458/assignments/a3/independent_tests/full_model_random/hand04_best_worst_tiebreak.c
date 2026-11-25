#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    void *a1 = my_malloc(216);
    void *p1 = my_malloc(10);
    void *a2 = my_malloc(216);
    void *p2 = my_malloc(10);
    void *a3 = my_malloc(344);
    void *p3 = my_malloc(10);
    void *a4 = my_malloc(344);
    void *fill0 = my_malloc(63800);
    (void)p1; (void)p2; (void)p3; (void)fill0;
    my_free(a1);
    my_free(a2);
    my_free(a3);
    my_free(a4);
    my_set_policy(BEST_FIT);
    void *b1 = my_malloc(200);
    my_set_policy(WORST_FIT);
    void *w1 = my_malloc(100);
    (void)b1; (void)w1;
    my_heap_dump();
    return 0;
}
