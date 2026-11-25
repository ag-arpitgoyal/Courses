/* delay_realloc_move: realloc move with k>0 delays the old block (0xDE, WAF via old pointer, double free) */
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
    char *a, *a2, *a3, *b, *b2;
    my_set_delay(2);
    a = my_malloc(10);
    b = my_malloc(10);
    memset(a, 0x61, 10);
    a2 = my_realloc(a, 100);
    dumpbytes(a2, 10);
    dumpbytes(a, 10);
    ((unsigned char *)a)[4] = 0x00;
    my_heap_dump();
    b2 = my_realloc(b, 300);
    a3 = my_realloc(a2, 2000);
    my_heap_dump();
    my_free(a);
    my_free(a2);
    my_set_delay(0);
    my_heap_dump();
    (void)a; (void)a2; (void)a3; (void)b; (void)b2;
    return 0;
}
