#include "myalloc.h"
#include <stdio.h>

int main(void) {
    void *p1 = my_malloc(1);
    void *p88 = my_malloc(88);
    void *p89 = my_malloc(89);
    void *p104 = my_malloc(104);
    void *p105 = my_malloc(105);
    (void)p1; (void)p88; (void)p89; (void)p104; (void)p105;
    my_heap_dump();
    return 0;
}
