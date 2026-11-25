/* guard_realloc_huge: clar. 3: overflow + my_realloc(p, 2^30+1) reports overflow, returns NULL, block unchanged */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *q;
    a = my_malloc(10);
    b = my_malloc(10);
    ((unsigned char *)a)[10] = 0x00;
    q = my_realloc(a, (1u << 30) + 1);
    printf("q %s\n", q ? "ptr" : "NULL");
    q = my_realloc(b, (1u << 30) + 1);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_heap_dump();
    my_leak_report();
    (void)a; (void)b; (void)q;
    return 0;
}
