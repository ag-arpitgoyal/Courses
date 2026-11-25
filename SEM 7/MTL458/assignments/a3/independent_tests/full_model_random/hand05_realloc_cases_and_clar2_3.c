#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    unsigned char *a = my_malloc(344);
    for (int i = 0; i < 344; i++) a[i] = (unsigned char)(i & 0xFF);
    void *b = my_malloc(10);
    unsigned char *c = my_malloc(216);
    for (int i = 0; i < 216; i++) c[i] = (unsigned char)((i * 3) & 0xFF);
    void *d = my_malloc(10);
    (void)d;
    my_free(b);
    a = my_realloc(a, 10);
    for (int i = 0; i < 10; i++) if (a[i] != (unsigned char)(i & 0xFF)) printf("CORRUPT a1\n");
    a = my_realloc(a, 200);
    for (int i = 0; i < 10; i++) if (a[i] != (unsigned char)(i & 0xFF)) printf("CORRUPT a2\n");
    my_set_policy(NEXT_FIT);
    void *e = my_malloc(10);
    (void)e;
    c[216] = 0x00;
    void *res = my_realloc(c, (1ULL << 30) + 100);
    if (res != NULL) printf("FAIL realloc huge\n");
    c[216] = 0xCA;
    my_set_policy(FIRST_FIT);
    c = my_realloc(c, 300);
    for (int i = 0; i < 216; i++) if (c[i] != (unsigned char)((i * 3) & 0xFF)) printf("CORRUPT c\n");
    my_heap_dump();
    return 0;
}
