/* guard_each_position: each of the 8 guard positions individually detected (n=24); untouched control block silent */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *p[9];
    p[0] = my_malloc(24);
    p[1] = my_malloc(24);
    p[2] = my_malloc(24);
    p[3] = my_malloc(24);
    p[4] = my_malloc(24);
    p[5] = my_malloc(24);
    p[6] = my_malloc(24);
    p[7] = my_malloc(24);
    p[8] = my_malloc(24);
    ((unsigned char *)p[0])[24] = 0x00;
    ((unsigned char *)p[1])[25] = 0xFF;
    ((unsigned char *)p[2])[26] = 0x41;
    ((unsigned char *)p[3])[27] = 0xCB;
    ((unsigned char *)p[4])[28] = 0xC9;
    ((unsigned char *)p[5])[29] = 0x01;
    ((unsigned char *)p[6])[30] = 0x80;
    ((unsigned char *)p[7])[31] = 0x7F;
    my_free(p[8]);
    my_free(p[0]);
    my_free(p[1]);
    my_free(p[2]);
    my_free(p[3]);
    my_free(p[4]);
    my_free(p[5]);
    my_free(p[6]);
    my_free(p[7]);
    my_heap_dump();
    (void)p;
    return 0;
}
