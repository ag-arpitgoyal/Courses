/* delay_never_coalesced: evicted blocks next to a delayed block do not merge with it; allocation cannot use the gap; draining merges */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *d, *e, *x;
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    d = my_malloc(10);
    e = my_malloc(10);
    my_set_delay(1);
    my_free(b);
    my_free(d);
    my_free(c);
    x = my_malloc(200);
    my_heap_dump();
    my_set_delay(0);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)x;
    return 0;
}
