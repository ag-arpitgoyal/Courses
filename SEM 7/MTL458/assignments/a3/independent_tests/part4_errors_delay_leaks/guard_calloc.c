/* guard_calloc: calloc guard sits after nmemb*size bytes */
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
    char *c1, *c2;
    c1 = my_calloc(4, 25);
    dumpbytes(c1 + 96, 4);
    dumpbytes(c1 + 100, 8);
    ((unsigned char *)c1)[104] = 0x00;
    my_free(c1);
    c2 = my_calloc(1, 88);
    ((unsigned char *)c2)[95] = 0x00;
    my_free(c2);
    my_heap_dump();
    (void)c1; (void)c2;
    return 0;
}
