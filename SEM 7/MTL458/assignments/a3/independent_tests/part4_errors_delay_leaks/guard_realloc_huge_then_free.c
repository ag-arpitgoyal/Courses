/* guard_realloc_huge_then_free: after failed my_realloc(p, 2^30+1) the modified guard is left as is, so my_free reports it again */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *q;
    a = my_malloc(10);
    ((unsigned char *)a)[12] = 0x00;
    q = my_realloc(a, (1u << 30) + 1);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_free(a);
    my_heap_dump();
    (void)a; (void)q;
    return 0;
}
