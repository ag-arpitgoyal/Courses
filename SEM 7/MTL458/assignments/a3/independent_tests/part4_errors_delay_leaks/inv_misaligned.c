/* inv_misaligned: misaligned pointers (p+2,+3,+4,+7) invalid via free and realloc */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *q;
    a = my_malloc(64);
    b = my_malloc(64);
    my_free(a + 4);
    my_free(a + 2);
    my_free(a + 7);
    my_free(a + 3);
    q = my_realloc(a + 4, 64);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_free(b + 1);
    my_heap_dump();
    my_free(a);
    my_free(b);
    my_heap_dump();
    (void)a; (void)b; (void)q;
    return 0;
}
