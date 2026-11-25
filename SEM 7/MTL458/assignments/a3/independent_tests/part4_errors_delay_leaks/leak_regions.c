/* leak_regions: leaks listed in heap order across regions (region 0 blocks allocated later come first) */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *d, *e, *g;
    a = my_malloc(100000);
    b = my_malloc(10);
    c = my_malloc(70000);
    d = my_malloc(30000);
    e = my_malloc(20);
    g = my_malloc(5000);
    my_free(b);
    my_leak_report();
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)g;
    return 0;
}
