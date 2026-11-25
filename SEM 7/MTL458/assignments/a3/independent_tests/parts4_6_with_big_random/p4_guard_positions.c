/* each of the 8 guard positions detected (various n); writes within n not detected */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *a0, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8;

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
    a0 = my_malloc((size_t)10ull);
    a1 = my_malloc((size_t)24ull);
    a2 = my_malloc((size_t)1ull);
    a3 = my_malloc((size_t)100ull);
    a4 = my_malloc((size_t)88ull);
    a5 = my_malloc((size_t)33ull);
    a6 = my_malloc((size_t)7ull);
    a7 = my_malloc((size_t)120ull);
    a8 = my_malloc((size_t)50ull);
    my_heap_dump();
    a0[10] = 0x00;
    a1[25] = 0x00;
    a2[3] = 0xFF;
    a3[103] = 0x00;
    a4[92] = 0xCB;
    a5[38] = 0x00;
    a6[13] = 0x41;
    a7[127] = 0x00;
    a8[0] = 0x00;
    a8[49] = 0x00;
    my_free(a0);
    my_free(a1);
    my_free(a2);
    my_free(a3);
    my_free(a4);
    my_free(a5);
    my_free(a6);
    my_free(a7);
    my_free(a8);
    my_heap_dump();
    return 0;
}
