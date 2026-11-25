/* delay_k1: k=1: each free evicts the previous one; evicted blocks coalesce; set_delay(0) drains */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c;
    my_set_delay(1);
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    my_free(a);
    my_heap_dump();
    my_free(b);
    my_heap_dump();
    my_free(c);
    my_heap_dump();
    my_set_delay(0);
    my_heap_dump();
    (void)a; (void)b; (void)c;
    return 0;
}
