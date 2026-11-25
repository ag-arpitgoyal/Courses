/* guard_various_n: guard sits right after n for many n (1,7,8,88,89,100,104,1000), incl. guard ending exactly at block end */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *q[9];
    q[0] = my_malloc(1);
    q[1] = my_malloc(7);
    q[2] = my_malloc(8);
    q[3] = my_malloc(88);
    q[4] = my_malloc(89);
    q[5] = my_malloc(100);
    q[6] = my_malloc(104);
    q[7] = my_malloc(1000);
    q[8] = my_malloc(50);
    my_heap_dump();
    memset(q[8], 0x77, 50);
    ((unsigned char *)q[0])[1] = 0x00;
    ((unsigned char *)q[1])[14] = 0x00;
    ((unsigned char *)q[2])[11] = 0x00;
    ((unsigned char *)q[3])[95] = 0x00;
    ((unsigned char *)q[4])[89] = 0x00;
    ((unsigned char *)q[5])[105] = 0x00;
    ((unsigned char *)q[6])[111] = 0x00;
    ((unsigned char *)q[7])[1004] = 0x00;
    my_free(q[8]);
    my_free(q[7]);
    my_free(q[6]);
    my_free(q[5]);
    my_free(q[4]);
    my_free(q[3]);
    my_free(q[2]);
    my_free(q[1]);
    my_free(q[0]);
    my_heap_dump();
    (void)q;
    return 0;
}
