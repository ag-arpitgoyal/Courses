/* guard_padding_not_checked: bytes in padding after the guard are not checked (n=10 and n=89); control overflow detected */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c;
    a = my_malloc(10);
    ((unsigned char *)a)[18] = 0x00;
    ((unsigned char *)a)[50] = 0x11;
    ((unsigned char *)a)[95] = 0xFF;
    my_free(a);
    b = my_malloc(89);
    ((unsigned char *)b)[97] = 0x00;
    ((unsigned char *)b)[111] = 0x00;
    my_free(b);
    c = my_malloc(10);
    ((unsigned char *)c)[17] = 0x00;
    my_free(c);
    my_heap_dump();
    (void)a; (void)b; (void)c;
    return 0;
}
