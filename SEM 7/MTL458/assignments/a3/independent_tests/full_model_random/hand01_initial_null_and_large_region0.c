#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    if (my_malloc(0) != NULL) printf("FAIL 1\n");
    if (my_malloc((1ULL << 30) + 1) != NULL) printf("FAIL 2\n");
    if (my_calloc(0, 100) != NULL) printf("FAIL 3\n");
    if (my_calloc(100, 0) != NULL) printf("FAIL 4\n");
    if (my_calloc(1ULL << 40, 1ULL << 30) != NULL) printf("FAIL 5\n");
    my_free(NULL);
    my_heap_dump();
    void *p = my_malloc(100000);
    if (p == NULL) printf("FAIL 6\n");
    my_heap_dump();
    my_leak_report();
    return 0;
}
