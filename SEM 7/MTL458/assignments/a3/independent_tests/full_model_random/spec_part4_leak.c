#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    void *a = my_malloc(100);
    void *b = my_malloc(10);
    void *c = my_malloc(10);
    (void)a; (void)c;
    my_free(b);
    my_leak_report();
    return 0;
}
