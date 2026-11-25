/* delay_blocks_grow: a delayed next block prevents grow in place (move instead); once free, grow in place works */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *a2, *b, *c, *c2;
    my_set_delay(1);
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    my_free(b);
    a2 = my_realloc(a, 100);
    my_heap_dump();
    my_set_delay(0);
    my_free(a2);
    c2 = my_realloc(c, 200);
    printf("c2==c %d\n", c2 == c);
    my_heap_dump();
    (void)a; (void)a2; (void)b; (void)c; (void)c2;
    return 0;
}
