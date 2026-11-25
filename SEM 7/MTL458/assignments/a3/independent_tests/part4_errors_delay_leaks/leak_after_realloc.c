/* leak_after_realloc: leak report uses requested sizes after realloc shrink/move and calloc products */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *d, *q;
    a = my_malloc(100);
    b = my_malloc(10);
    a = my_realloc(a, 30);
    c = my_calloc(6, 7);
    b = my_realloc(b, 400);
    c = my_realloc(c, 60);
    d = my_malloc(33);
    q = my_realloc(d, 0);
    q = my_realloc(a, (1u << 30) + 1);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_leak_report();
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)d; (void)q;
    return 0;
}
