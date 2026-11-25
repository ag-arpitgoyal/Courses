/* ex_delay: task.md Part 4 delay-queue example verbatim */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *d;
    my_set_delay(2);
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    d = my_malloc(10);
    my_free(a);
    my_free(b);
    ((unsigned char *)b)[3] = 0x58;
    my_free(c);
    my_free(d);
    my_heap_dump();
    my_free(a);
    my_free(b);
    (void)a; (void)b; (void)c; (void)d;
    return 0;
}
