/* delay_double_free: freeing/reallocing a delayed block is a double free and leaves the queue untouched; middle of delayed block invalid */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *q;
    my_set_delay(2);
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    my_free(a);
    my_free(b);
    my_free(a);
    my_free(b);
    my_free(a + 5);
    q = my_realloc(a, 50);
    printf("q %s\n", q ? "ptr" : "NULL");
    q = my_realloc(b, 0);
    printf("q %s\n", q ? "ptr" : "NULL");
    q = my_realloc(a, (1u << 30) + 1);
    printf("q %s\n", q ? "ptr" : "NULL");
    ((unsigned char *)a)[0] = 0x00;
    my_free(c);
    my_free(a);
    my_heap_dump();
    my_set_delay(0);
    my_free(b);
    my_free(a);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)q;
    return 0;
}
