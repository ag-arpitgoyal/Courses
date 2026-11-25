/* delay_only_n_checked: eviction checks only the n bytes: writes into a delayed block's guard/padding are not reported; control write in n is */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *d;
    a = my_malloc(10);
    b = my_malloc(89);
    c = my_malloc(10);
    d = my_malloc(10);
    my_set_delay(3);
    my_free(a);
    my_free(b);
    my_free(d);
    ((unsigned char *)a)[10] = 0x00;
    ((unsigned char *)a)[95] = 0x11;
    ((unsigned char *)b)[89] = 0x00;
    ((unsigned char *)b)[111] = 0x00;
    ((unsigned char *)a)[9] = 0xDE;
    ((unsigned char *)d)[9] = 0x00;
    my_set_delay(0);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)d;
    return 0;
}
