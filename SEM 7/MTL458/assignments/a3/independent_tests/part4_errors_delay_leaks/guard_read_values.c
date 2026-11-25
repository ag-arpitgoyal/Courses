/* guard_read_values: guard bytes read back as 0xCA after malloc, calloc, realloc in place, move, grow in place; calloc zeroes; move copies data */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static void dumpbytes(const char *p, int len)
{
    for (int i = 0; i < len; i++)
        printf("%02x%c", (unsigned char)p[i], i + 1 < len ? ' ' : '\n');
}

int main(void)
{
    char *a, *c, *d;
    a = my_malloc(10);
    dumpbytes(a + 10, 8);
    c = my_calloc(3, 5);
    dumpbytes(c, 15);
    dumpbytes(c + 15, 8);
    a = my_realloc(a, 5);
    dumpbytes(a + 5, 8);
    a = my_realloc(a, 60);
    dumpbytes(a + 60, 8);
    memset(a, 0x5A, 60);
    a = my_realloc(a, 100);
    dumpbytes(a, 4);
    dumpbytes(a + 56, 4);
    dumpbytes(a + 100, 8);
    my_free(c);
    d = my_malloc(10);
    d = my_realloc(d, 100);
    dumpbytes(d + 100, 8);
    my_heap_dump();
    (void)a; (void)c; (void)d;
    return 0;
}
