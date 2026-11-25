#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    my_set_policy(BUDDY);
    if (my_malloc(0) != NULL) printf("FAIL 0\n");
    if (my_malloc(1048561) != NULL) printf("FAIL too big\n");
    my_heap_dump();
    void *full = my_malloc(1048560);
    if (full == NULL) printf("FAIL full\n");
    my_heap_dump();
    if (my_malloc(1) != NULL) printf("FAIL should be full\n");
    my_free(full);
    my_heap_dump();
    void *p0 = my_malloc(1);
    void *p1 = my_malloc(16);
    void *p2 = my_malloc(17);
    my_free(p0);
    my_free(p2);
    my_heap_dump();
    my_free(p1);
    my_heap_dump();
    return 0;
}
