/* err_no_alloc_nextfit: erroneous my_realloc calls neither allocate nor move the next-fit position */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char stk[64];
    char *a, *b, *c, *d, *q;
    memset(stk, 0, sizeof stk);
    my_set_policy(NEXT_FIT);
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    my_free(a);
    q = my_realloc(a, 10);
    q = my_realloc(b + 1, 10);
    q = my_realloc(stk, 10);
    d = my_malloc(10);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)d; (void)q;
    return 0;
}
