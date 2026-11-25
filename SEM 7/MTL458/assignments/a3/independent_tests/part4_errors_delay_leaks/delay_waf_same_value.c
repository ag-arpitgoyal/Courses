/* delay_waf_same_value: writing 0xDE (or restoring it) into a delayed block is not a write after free */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c;
    my_set_delay(2);
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    my_free(a);
    my_free(b);
    ((unsigned char *)a)[3] = 0xDE;
    ((unsigned char *)b)[4] = 0x00;
    ((unsigned char *)b)[4] = 0xDE;
    my_set_delay(0);
    my_heap_dump();
    (void)a; (void)b; (void)c;
    return 0;
}
