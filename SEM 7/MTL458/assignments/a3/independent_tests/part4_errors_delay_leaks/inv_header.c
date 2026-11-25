/* inv_header: pointers to block headers / region header area are invalid; b-128 equals a and frees it */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *base, *q;
    a = my_malloc(10);
    b = my_malloc(10);
    base = a - 64;
    my_free(a - 32);
    my_free(a - 16);
    my_free(a - 1);
    my_free(base);
    my_free(base + 16);
    my_free(base + 31);
    my_free(b - 32);
    my_free(b - 8);
    q = my_realloc(b - 32, 10);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_heap_dump();
    my_free(b - 128);
    my_free(a);
    my_heap_dump();
    (void)a; (void)b; (void)base; (void)q;
    return 0;
}
