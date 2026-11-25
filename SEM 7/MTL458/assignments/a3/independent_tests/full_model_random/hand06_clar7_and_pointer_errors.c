#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    char *a = my_malloc(10);
    char *b = my_malloc(10);
    int stack_var = 42;
    my_free(b + 128);
    my_free(a + 1);
    my_free(&stack_var);
    my_free(a - 32);
    my_free(a);
    my_free(b);
    my_free(a);
    my_free(b);
    my_heap_dump();
    return 0;
}
