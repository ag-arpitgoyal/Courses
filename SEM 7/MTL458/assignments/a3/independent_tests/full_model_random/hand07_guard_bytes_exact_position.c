#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    unsigned char *a = my_malloc(10);
    for (int i = 0; i < 8; i++) {
        if (a[10 + i] != 0xCA) printf("GUARD MISMATCH INIT %d\n", i);
    }
    a[17] = 0xCB;
    a = my_realloc(a, 25);
    for (int i = 0; i < 8; i++) {
        if (a[25 + i] != 0xCA) printf("GUARD MISMATCH REALLOC %d\n", i);
    }
    a[25] = 0x00;
    my_free(a);
    my_heap_dump();
    return 0;
}
