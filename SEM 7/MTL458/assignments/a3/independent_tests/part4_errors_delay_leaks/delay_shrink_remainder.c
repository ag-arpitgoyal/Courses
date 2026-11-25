/* delay_shrink_remainder: shrink-split remainder with k>0 is a plain free block (not delayed) and does not merge with a delayed next block */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *a2, *b, *c, *x;
    my_set_delay(2);
    a = my_malloc(300);
    b = my_malloc(10);
    c = my_malloc(10);
    my_free(b);
    a2 = my_realloc(a, 10);
    printf("a2==a %d\n", a2 == a);
    my_heap_dump();
    x = my_malloc(100);
    my_heap_dump();
    (void)a; (void)a2; (void)b; (void)c; (void)x;
    return 0;
}
