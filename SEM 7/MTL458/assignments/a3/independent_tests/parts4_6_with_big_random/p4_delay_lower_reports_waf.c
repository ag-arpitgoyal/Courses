/* lowering k evicts from the front and reports write-after-free of evicted blocks */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a, *b, *c;

static int allb(const unsigned char *p, int v, size_t c)
{
    for (size_t i = 0; i < c; i++)
        if (p[i] != (unsigned char)v)
            return 0;
    return 1;
}

int main(void)
{
    unsigned char STACK_BUF[64];
    memset(STACK_BUF, 0, sizeof STACK_BUF);
    (void)STATIC_BUF; (void)allb;
    my_set_delay(3);
    a = my_malloc((size_t)10ull);
    b = my_malloc((size_t)20ull);
    c = my_malloc((size_t)30ull);
    my_free(a);
    my_free(b);
    my_free(c);
    b[19] = 0x00;
    my_set_delay(1);
    my_heap_dump();
    return 0;
}
