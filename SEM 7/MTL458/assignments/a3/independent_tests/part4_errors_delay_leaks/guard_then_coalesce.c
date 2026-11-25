/* guard_then_coalesce: overflow is reported and the block is still freed and coalesced on both sides */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c;
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    my_free(a);
    my_free(c);
    ((unsigned char *)b)[15] = 0x00;
    my_free(b);
    my_heap_dump();
    (void)a; (void)b; (void)c;
    return 0;
}
