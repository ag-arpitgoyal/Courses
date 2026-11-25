/* err_does_nothing_more: after invalid/double-free errors nothing else happens: no guard check, no queueing, no eviction */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char stk[64];
    char *a, *b, *q;
    memset(stk, 0, sizeof stk);
    a = my_malloc(10);
    b = my_malloc(10);
    ((unsigned char *)a)[10] = 0x00;
    my_free(a + 1);
    q = my_realloc(a + 1, 20);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_heap_dump();
    my_set_delay(1);
    my_free(b);
    ((unsigned char *)b)[0] = 0x58;
    my_free(b);
    q = my_realloc(b, 0);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_free(stk);
    my_heap_dump();
    my_free(a);
    my_heap_dump();
    my_set_delay(0);
    my_heap_dump();
    (void)a; (void)b; (void)q;
    return 0;
}
