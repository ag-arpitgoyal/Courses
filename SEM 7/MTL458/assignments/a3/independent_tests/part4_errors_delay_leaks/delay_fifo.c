/* delay_fifo: k=3 FIFO order independent of heap order; lowering to 1 then 0 evicts from the front */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *p0, *p1, *p2, *p3, *p4, *p5;
    my_set_delay(3);
    p0 = my_malloc(10);
    p1 = my_malloc(10);
    p2 = my_malloc(10);
    p3 = my_malloc(10);
    p4 = my_malloc(10);
    p5 = my_malloc(10);
    my_free(p2);
    my_free(p0);
    ((unsigned char *)p0)[5] = 0x00;
    my_free(p4);
    my_free(p1);
    ((unsigned char *)p1)[0] = 0x41;
    my_free(p5);
    my_heap_dump();
    my_free(p3);
    ((unsigned char *)p3)[9] = 0x00;
    my_set_delay(1);
    my_heap_dump();
    my_set_delay(0);
    my_heap_dump();
    (void)p0; (void)p1; (void)p2; (void)p3; (void)p4; (void)p5;
    return 0;
}
