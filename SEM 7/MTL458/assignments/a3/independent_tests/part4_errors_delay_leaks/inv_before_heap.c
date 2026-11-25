/* inv_before_heap: invalid pointers / NULL-returning calls before any region exists create no region */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char gbuf[256];

int main(void)
{
    char stk[64];
    char *a, *q, *q2, *q3, *q4;
    memset(stk, 0, sizeof stk);
    my_free(stk);
    q = my_realloc(stk, 10);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_free(gbuf);
    my_free(NULL);
    q2 = my_realloc(NULL, 0);
    printf("q2 %s\n", q2 ? "ptr" : "NULL");
    q3 = my_malloc(0);
    printf("q3 %s\n", q3 ? "ptr" : "NULL");
    q4 = my_realloc(NULL, (1u << 30) + 1);
    printf("q4 %s\n", q4 ? "ptr" : "NULL");
    my_heap_dump();
    printf("nothing above\n");
    a = my_malloc(10);
    my_heap_dump();
    (void)a; (void)q; (void)q2; (void)q3; (void)q4;
    return 0;
}
