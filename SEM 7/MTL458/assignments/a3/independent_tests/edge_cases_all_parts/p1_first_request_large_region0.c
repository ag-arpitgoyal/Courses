#include "myalloc.h"
#include <stdio.h>

int main(void) {
    void *a = my_malloc(100000);
    (void)a;
    my_heap_dump();
    return 0;
}
