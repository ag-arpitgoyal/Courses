/* dbl_merge_forward: a freed block that absorbs its successor keeps its start (double free); absorbed starts become invalid */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c;
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    my_free(c);
    my_free(b);
    my_free(c);
    my_free(b);
    my_free(a);
    my_free(b);
    my_free(a);
    my_heap_dump();
    (void)a; (void)b; (void)c;
    return 0;
}
