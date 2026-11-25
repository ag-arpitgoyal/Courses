/* guard_realloc_move: after a move the guard follows n in the new block; old pointer is then a double free */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *p, *p2, *q;
    p = my_malloc(10);
    q = my_malloc(10);
    p2 = my_realloc(p, 200);
    ((unsigned char *)p2)[200] = 0x00;
    my_free(p2);
    my_free(p);
    my_heap_dump();
    (void)p; (void)p2; (void)q;
    return 0;
}
