/* delay_raise_k: raising k never evicts; later frees accumulate up to the new k */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *d, *e;
    my_set_delay(1);
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    d = my_malloc(10);
    e = my_malloc(10);
    my_free(a);
    my_set_delay(3);
    my_free(b);
    my_free(c);
    my_heap_dump();
    my_free(d);
    ((unsigned char *)b)[0] = 0x00;
    my_set_delay(64);
    my_free(e);
    my_heap_dump();
    my_set_delay(0);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)d; (void)e;
    return 0;
}
