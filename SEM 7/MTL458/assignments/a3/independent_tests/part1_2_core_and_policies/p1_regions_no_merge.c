/* p1_regions_no_merge: free blocks in adjacent regions never merge; a request larger than any single free block grows the heap; reuse of whole regions */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *f, *d, *e;

static void chk(const char *name, const char *p, size_t n, unsigned char t)
{
    size_t i;
    for (i = 0; i < n; i++)
        if ((unsigned char)p[i] != t) { printf("%s bad\n", name); return; }
    printf("%s ok\n", name);
}

int main(void)
{
    (void)chk;
    a = my_malloc((size_t)65464);
    b = my_malloc((size_t)65464);
    c = my_malloc((size_t)10);
    my_free(a);
    my_free(b);
    my_heap_dump();
    f = my_malloc((size_t)70000);
    my_heap_dump();
    d = my_malloc((size_t)65464);
    e = my_malloc((size_t)65400);
    my_heap_dump();
    return 0;
}
