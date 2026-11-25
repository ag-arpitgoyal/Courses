/* leak_none: all freed -> summary only with zeros */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b;
    a = my_malloc(10);
    b = my_calloc(3, 3);
    my_free(a);
    my_free(b);
    my_leak_report();
    my_heap_dump();
    (void)a; (void)b;
    return 0;
}
