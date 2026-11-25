/* guard_free_order: overflow messages follow call order, not heap order; untouched block silent */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *d;
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    d = my_malloc(10);
    ((unsigned char *)b)[12] = 0x00;
    ((unsigned char *)d)[16] = 0x00;
    ((unsigned char *)a)[11] = 0x00;
    my_free(d);
    my_free(c);
    my_free(a);
    my_free(b);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)d;
    return 0;
}
