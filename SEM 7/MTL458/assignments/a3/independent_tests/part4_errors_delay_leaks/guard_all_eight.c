/* guard_all_eight: all 8 guard bytes changed -> exactly one message, via free, realloc n=0 and realloc move */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *c2, *d, *q;
    a = my_malloc(40);
    b = my_malloc(40);
    memset(a + 40, 0x00, 8);
    memset(b + 40, 0xFF, 8);
    my_free(a);
    q = my_realloc(b, 0);
    printf("q %s\n", q ? "ptr" : "NULL");
    c = my_malloc(40);
    d = my_malloc(40);
    memset(c + 40, 0x11, 8);
    c2 = my_realloc(c, 500);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)c2; (void)d; (void)q;
    return 0;
}
