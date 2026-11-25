#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static void fill2(unsigned char *p, size_t n) {
    for (size_t i = 0; i < n; i++) p[i] = (unsigned char)((i * 31u) % 251u);
}
static int same2(const unsigned char *p, size_t n) {
    for (size_t i = 0; i < n; i++)
        if (p[i] != (unsigned char)((i * 31u) % 251u)) return 0;
    return 1;
}

int main(void) {
    char *a = my_malloc(27);
    memcpy(a, "abcdefghijklmnopqrstuvwxyz", 27);
    char *b = my_malloc(10);
    memcpy(b, "123456789", 10);

    a = my_realloc(a, 60);          /* in place, same block */
    printf("%s\n", a);
    a = my_realloc(a, 200);         /* move */
    printf("%s\n", a);
    memcpy(a + 26, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 27);
    a = my_realloc(a, 1000);        /* grow in place */
    printf("%s\n", a);
    a = my_realloc(a, 10);          /* shrink in place with split */
    printf("%.10s\n", a);
    b = my_realloc(b, 100);         /* move */
    printf("%s\n", b);
    my_heap_dump();

    unsigned char *c = my_malloc(5000);
    fill2(c, 5000);
    c = my_realloc(c, 2500);        /* shrink, tail merges with free space */
    printf("c %d %02x %02x\n", same2(c, 2500), c[0], c[2499]);
    c = my_realloc(c, 7000);        /* grow in place */
    printf("c %d %02x %02x\n", same2(c, 2500), c[1], c[2499]);
    fill2(c, 7000);
    c = my_realloc(c, 60000);       /* grow in place */
    printf("c %d %02x\n", same2(c, 7000), c[6999]);
    fill2(c, 60000);
    c = my_realloc(c, 64000);       /* grow in place */
    printf("c %d %02x\n", same2(c, 60000), c[59999]);
    fill2(c, 64000);
    c = my_realloc(c, 64937);       /* move to a new region */
    printf("c %d %02x %02x\n", same2(c, 64000), c[0], c[63999]);
    my_heap_dump();
    my_leak_report();
    return 0;
}
