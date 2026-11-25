/* delay_realloc_n0: my_realloc(p,0) with k>0 delays the block; later double frees; WAF on drain */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *q;
    my_set_delay(2);
    a = my_malloc(10);
    b = my_malloc(20);
    c = my_malloc(10);
    q = my_realloc(a, 0);
    printf("q %s\n", q ? "ptr" : "NULL");
    q = my_realloc(b, 0);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_heap_dump();
    my_free(a);
    q = my_realloc(b, 0);
    printf("q %s\n", q ? "ptr" : "NULL");
    ((unsigned char *)b)[19] = 0x00;
    q = my_realloc(c, 0);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_set_delay(0);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)q;
    return 0;
}
