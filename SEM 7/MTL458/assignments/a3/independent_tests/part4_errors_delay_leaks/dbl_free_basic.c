/* dbl_free_basic: double free of a free block via my_free and my_realloc (n=50, n=0, n>2^30) */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *q;
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    my_free(a);
    my_free(a);
    my_free(c);
    my_free(c);
    q = my_realloc(a, 20);
    printf("q %s\n", q ? "ptr" : "NULL");
    q = my_realloc(c, 0);
    printf("q %s\n", q ? "ptr" : "NULL");
    q = my_realloc(a, (1u << 30) + 1);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_heap_dump();
    my_leak_report();
    (void)a; (void)b; (void)c; (void)q;
    return 0;
}
