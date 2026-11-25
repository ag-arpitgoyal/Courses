#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    my_set_delay(4);
    unsigned char *a = my_malloc(10);
    unsigned char *b = my_malloc(10);
    unsigned char *c = my_malloc(10);
    unsigned char *d = my_malloc(10);
    unsigned char *e = my_malloc(10);
    (void)e;
    my_free(a);
    void *r0 = my_realloc(b, 0);
    if (r0 != NULL) printf("FAIL realloc 0\n");
    a[0] = 0x99;
    unsigned char *c_new = my_realloc(c, 200);
    (void)c_new;
    c[9] = 0x11;
    my_free(d);
    my_set_delay(1);
    my_heap_dump();
    my_leak_report();
    return 0;
}
