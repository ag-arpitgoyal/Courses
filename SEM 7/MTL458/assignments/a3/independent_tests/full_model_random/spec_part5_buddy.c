#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    my_set_policy(BUDDY);
    void *a = my_malloc(40);
    void *b = my_malloc(100);
    void *c = my_malloc(40);
    (void)a; (void)b; (void)c;
    my_heap_dump();
    return 0;
}
