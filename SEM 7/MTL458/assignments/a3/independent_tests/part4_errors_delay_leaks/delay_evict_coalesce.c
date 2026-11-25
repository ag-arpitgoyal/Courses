/* delay_evict_coalesce: an evicted block coalesces with free blocks on both sides, then with the tail */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *x, *y;
    x = my_malloc(10);
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    y = my_malloc(10);
    my_free(a);
    my_free(c);
    my_set_delay(1);
    my_free(b);
    my_heap_dump();
    my_free(y);
    my_heap_dump();
    my_set_delay(0);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)x; (void)y;
    return 0;
}
