/* p1_pointer_layout: returned pointer = header+32, so pointer differences equal block sizes; pointers 16-aligned; user data is preserved */
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
    b = my_malloc((size_t)89);
    c = my_malloc((size_t)5000);
    d = my_malloc((size_t)1);
    printf("b-a %ld\n", (long)(b - a));
    printf("c-b %ld\n", (long)(c - b));
    printf("d-c %ld\n", (long)(d - c));
    printf("align a %d\n", (int)((uintptr_t)a % 16));
    printf("align c %d\n", (int)((uintptr_t)c % 16));
    memset(a, 0x41, 10);
    memset(b, 0x42, 89);
    memset(c, 0x43, 5000);
    memset(d, 0x44, 1);
    chk("a", a, 10, 0x41);
    chk("b", b, 89, 0x42);
    chk("c", c, 5000, 0x43);
    chk("d", d, 1, 0x44);
    my_heap_dump();
    return 0;
}
