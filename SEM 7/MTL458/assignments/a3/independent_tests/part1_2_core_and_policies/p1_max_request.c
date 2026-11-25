/* p1_max_request: n == 2^30 is allowed (region 1073807360), 2^30+1 is NULL and creates no region; free and coalesce of the huge block */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *big, *s;

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
    printf("malloc(1073741825) %s\n", my_malloc((size_t)1073741825) ? "ptr" : "NULL");
    my_heap_dump();
    big = my_malloc((size_t)1073741824);
    s = my_malloc((size_t)10);
    my_heap_dump();
    my_free(big);
    my_heap_dump();
    my_free(s);
    my_heap_dump();
    return 0;
}
