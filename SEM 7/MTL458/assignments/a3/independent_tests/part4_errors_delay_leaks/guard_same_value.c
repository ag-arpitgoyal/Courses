/* guard_same_value: writing 0xCA into guard (or restoring it) is not an overflow; 0xCB is */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c;
    a = my_malloc(10);
    ((unsigned char *)a)[10] = 0xCA;
    ((unsigned char *)a)[17] = 0xCA;
    my_free(a);
    b = my_malloc(10);
    ((unsigned char *)b)[12] = 0xCB;
    my_free(b);
    c = my_malloc(10);
    ((unsigned char *)c)[13] = 0x00;
    ((unsigned char *)c)[13] = 0xCA;
    my_free(c);
    my_heap_dump();
    (void)a; (void)b; (void)c;
    return 0;
}
