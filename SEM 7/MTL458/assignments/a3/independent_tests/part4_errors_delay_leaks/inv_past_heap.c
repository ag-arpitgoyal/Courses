/* inv_past_heap: pointers past / before the heap are invalid; leftover free block data start is a double free (clar. 7); region 1 too */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *base, *base1, *c, *q;
    a = my_malloc(10);
    base = a - 64;
    my_free(base + 65568);
    my_free(base + 65536);
    my_free(base + 65528);
    my_free(base + 69632);
    my_free(base - 4096);
    my_free(a + 128);
    c = my_malloc(70000);
    base1 = c - 64;
    my_free(base1 + 131104);
    my_free(c + 70048);
    my_free(base1 + 32);
    q = my_realloc(base1 + 131104, 10);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_heap_dump();
    (void)a; (void)base; (void)base1; (void)c; (void)q;
    return 0;
}
