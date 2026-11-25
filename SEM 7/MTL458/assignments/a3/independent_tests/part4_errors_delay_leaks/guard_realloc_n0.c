/* guard_realloc_n0: overflow + my_realloc(p,0): one message, block freed (k=0) or delayed (k=3) */
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
    char *a, *b, *c, *q;
    a = my_malloc(10);
    b = my_malloc(10);
    ((unsigned char *)a)[11] = 0x00;
    q = my_realloc(a, 0);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_free(a);
    my_set_delay(3);
    c = my_malloc(10);
    ((unsigned char *)c)[16] = 0x01;
    q = my_realloc(c, 0);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_heap_dump();
    dumpbytes(c, 10);
    my_free(c);
    (void)a; (void)b; (void)c; (void)q;
    return 0;
}
