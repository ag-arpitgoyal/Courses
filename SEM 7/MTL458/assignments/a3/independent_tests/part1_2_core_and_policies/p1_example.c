/* p1_example: worked example of Part 1 in task.md */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *d;

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
    a = my_malloc((size_t)10);
    b = my_malloc((size_t)100);
    c = my_malloc((size_t)10);
    my_heap_dump();
    my_free(b);
    my_free(a);
    d = my_malloc((size_t)100000);
    my_heap_dump();
    return 0;
}
