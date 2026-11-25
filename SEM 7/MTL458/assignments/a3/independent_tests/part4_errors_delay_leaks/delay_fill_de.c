/* delay_fill_de: delayed blocks have their n bytes set to 0xDE (malloc, calloc, realloc move old block); D lines; leak excludes D */
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
    char *a, *b, *c, *d, *d2, *e;
    my_set_delay(4);
    a = my_malloc(10);
    b = my_malloc(100);
    c = my_calloc(2, 7);
    memset(a, 0x11, 10);
    memset(b, 0x22, 100);
    my_free(a);
    dumpbytes(a, 10);
    my_free(b);
    dumpbytes(b, 100);
    my_free(c);
    dumpbytes(c, 14);
    d = my_malloc(20);
    memset(d, 0x44, 20);
    e = my_malloc(10);
    d2 = my_realloc(d, 200);
    dumpbytes(d, 20);
    dumpbytes(d2, 20);
    my_heap_dump();
    my_leak_report();
    (void)a; (void)b; (void)c; (void)d; (void)d2; (void)e;
    return 0;
}
