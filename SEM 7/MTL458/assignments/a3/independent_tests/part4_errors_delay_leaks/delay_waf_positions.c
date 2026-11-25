/* delay_waf_positions: write after free at first/middle/last byte, several bytes (one message), whole block; clean block silent */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *d, *e, *g;
    my_set_delay(8);
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    d = my_malloc(100);
    e = my_malloc(10);
    g = my_malloc(10);
    my_free(a);
    my_free(b);
    my_free(c);
    my_free(d);
    my_free(e);
    my_free(g);
    ((unsigned char *)a)[0] = 0x00;
    ((unsigned char *)b)[5] = 0x41;
    ((unsigned char *)c)[9] = 0xFF;
    ((unsigned char *)d)[0] = 0x01;
    ((unsigned char *)d)[50] = 0x02;
    ((unsigned char *)d)[99] = 0x03;
    memset(e, 0x00, 10);
    my_set_delay(0);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)g;
    return 0;
}
