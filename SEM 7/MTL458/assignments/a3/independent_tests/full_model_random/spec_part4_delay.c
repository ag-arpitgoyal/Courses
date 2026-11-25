#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    my_set_delay(2);
    char *a = my_malloc(10);
    char *b = my_malloc(10);
    char *c = my_malloc(10);
    char *d = my_malloc(10);
    my_free(a);
    my_free(b);
    b[3] = 'X';
    my_free(c);
    my_free(d);
    my_heap_dump();
    my_free(a);
    my_free(b);
    return 0;
}
