/* inv_middle: pointers into the middle of an allocated block (incl. guard start, last byte) are invalid; block data/guard intact */
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
    char *a, *b, *q;
    a = my_malloc(100);
    b = my_malloc(10);
    memset(a, 0x11, 100);
    my_free(a + 1);
    my_free(a + 8);
    my_free(a + 16);
    my_free(a + 50);
    my_free(a + 99);
    my_free(a + 100);
    my_free(a + 111);
    my_free(b + 64);
    q = my_realloc(a + 32, 20);
    printf("q %s\n", q ? "ptr" : "NULL");
    q = my_realloc(b + 1, 0);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_heap_dump();
    dumpbytes(a + 96, 4);
    dumpbytes(a + 100, 8);
    my_free(a);
    my_free(b);
    my_heap_dump();
    (void)a; (void)b; (void)q;
    return 0;
}
