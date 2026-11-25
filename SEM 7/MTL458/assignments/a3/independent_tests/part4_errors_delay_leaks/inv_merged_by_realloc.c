/* inv_merged_by_realloc: stale pointer absorbed by grow-in-place is invalid; after shrink-split it is a free block start again (double free) */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *p, *q, *r;
    p = my_malloc(10);
    q = my_malloc(10);
    r = my_malloc(10);
    my_free(q);
    p = my_realloc(p, 100);
    my_free(q);
    my_heap_dump();
    p = my_realloc(p, 10);
    my_free(q);
    my_heap_dump();
    (void)p; (void)q; (void)r;
    return 0;
}
