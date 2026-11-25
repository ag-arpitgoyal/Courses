#include "myalloc.h"
#include <stdio.h>

int main(void) {
    void *a = my_malloc(65464);
    my_heap_dump();
    my_free(a);
    void *b = my_malloc(65465);
    my_heap_dump();
    (void)b;
    return 0;
}
