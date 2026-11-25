/* guard_realloc_move_once: overflow on a moved realloc is reported exactly once (k=0 and k=2); data copied */
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
    char *a, *a2, *b, *c, *c2;
    a = my_malloc(10);
    b = my_malloc(10);
    memset(a, 0x61, 10);
    ((unsigned char *)a)[10] = 0x00;
    ((unsigned char *)a)[17] = 0x00;
    a2 = my_realloc(a, 1000);
    dumpbytes(a2, 10);
    my_heap_dump();
    my_free(a2);
    my_set_delay(2);
    c = my_malloc(10);
    ((unsigned char *)c)[13] = 0x00;
    c2 = my_realloc(c, 500);
    my_heap_dump();
    my_set_delay(0);
    my_free(c2);
    my_heap_dump();
    (void)a; (void)a2; (void)b; (void)c; (void)c2;
    return 0;
}
