/* delay_leak: leak report excludes delayed and free blocks */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *d;
    my_set_delay(3);
    a = my_malloc(100);
    b = my_malloc(10);
    c = my_malloc(50);
    d = my_malloc(7);
    my_free(b);
    my_free(d);
    my_leak_report();
    my_set_delay(0);
    my_free(a);
    my_leak_report();
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)d;
    return 0;
}
