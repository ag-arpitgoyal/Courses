/* inv_stack_global: stack, global and static-local pointers to my_free/my_realloc are invalid; heap untouched */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char gbuf[256];

int main(void)
{
    char stk[64];
    static char sloc[64];
    char *a, *b, *q;
    memset(stk, 0, sizeof stk);
    a = my_malloc(10);
    b = my_malloc(100);
    my_free(stk);
    my_free(stk + 32);
    my_free(gbuf);
    my_free(gbuf + 64);
    my_free(sloc);
    q = my_realloc(stk, 50);
    printf("q %s\n", q ? "ptr" : "NULL");
    q = my_realloc(gbuf, 0);
    printf("q %s\n", q ? "ptr" : "NULL");
    q = my_realloc(sloc, (1u << 30) + 1);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_heap_dump();
    my_leak_report();
    (void)a; (void)b; (void)q;
    return 0;
}
