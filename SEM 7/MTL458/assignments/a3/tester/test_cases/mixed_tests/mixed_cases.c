#include <stdio.h>
#include <string.h>

#include "myalloc.h"

int main(void) {
    my_set_policy(NEXT_FIT);
    my_set_delay(1);
    unsigned char *a = my_malloc(20);
    void *b = my_malloc(20);
    unsigned char *c = my_calloc(2, 10);
    if (!a || !b || !c) return 2;
    memcpy(c, "allocator-mix", 13);
    my_free(a);
    c = my_realloc(c, 1000);
    if (!c || memcmp(c, "allocator-mix", 13) != 0) return 3;
    my_free(b);
    my_heap_dump();
    my_leak_report();
    return 0;
}
