/* guard_before_eviction: guard check is printed before the write-after-free of the block it evicts (my_free and realloc move) */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *c2, *d;
    my_set_delay(1);
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    d = my_malloc(10);
    my_free(a);
    ((unsigned char *)a)[0] = 0x58;
    ((unsigned char *)b)[10] = 0x00;
    my_free(b);
    ((unsigned char *)c)[10] = 0x00;
    ((unsigned char *)b)[1] = 0x00;
    c2 = my_realloc(c, 300);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)c2; (void)d;
    return 0;
}
