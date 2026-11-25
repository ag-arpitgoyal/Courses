/* p1_first_big: clarification 1: first request larger than 65536-32 sizes region 0 by the growth rule; later small request goes into its leftover */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b;

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
    a = my_malloc((size_t)100000);
    my_heap_dump();
    b = my_malloc((size_t)10);
    my_heap_dump();
    return 0;
}
