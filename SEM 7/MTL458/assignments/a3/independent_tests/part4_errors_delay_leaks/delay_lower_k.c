/* delay_lower_k: lowering k evicts from the front with write-after-free checks; same k is a no-op */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *p0, *p1, *p2, *p3, *p4;
    my_set_delay(4);
    p0 = my_malloc(10);
    p1 = my_malloc(10);
    p2 = my_malloc(10);
    p3 = my_malloc(10);
    p4 = my_malloc(10);
    my_free(p0);
    my_free(p1);
    my_free(p2);
    my_free(p3);
    ((unsigned char *)p0)[0] = 0x00;
    ((unsigned char *)p2)[2] = 0x00;
    my_set_delay(2);
    my_heap_dump();
    my_set_delay(2);
    my_set_delay(1);
    my_free(p4);
    my_heap_dump();
    my_set_delay(0);
    my_heap_dump();
    (void)p0; (void)p1; (void)p2; (void)p3; (void)p4;
    return 0;
}
