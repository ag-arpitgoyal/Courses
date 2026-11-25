/* delay_set0_immediate: after my_set_delay(0) drains, frees are immediate again */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c;
    my_set_delay(3);
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    my_free(a);
    my_set_delay(0);
    my_free(b);
    my_heap_dump();
    (void)a; (void)b; (void)c;
    return 0;
}
