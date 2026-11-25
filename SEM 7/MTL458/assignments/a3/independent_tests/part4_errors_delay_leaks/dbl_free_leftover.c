/* dbl_free_leftover: clar. 7: data address of split leftovers (malloc split, realloc shrink split, grow-in-place split) is a double free */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *p, *q, *z;
    a = my_malloc(10);
    my_free(a + 128);
    b = my_malloc(100);
    my_free(b + 144);
    p = my_malloc(300);
    q = my_malloc(10);
    p = my_realloc(p, 10);
    my_free(p + 128);
    z = my_realloc(p + 128, 50);
    printf("z %s\n", z ? "ptr" : "NULL");
    q = my_realloc(q, 200);
    my_free(q + 240);
    my_heap_dump();
    (void)a; (void)b; (void)p; (void)q; (void)z;
    return 0;
}
