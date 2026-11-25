#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static void fill(void *p, size_t n, unsigned seed) {
    unsigned char *c = p;
    for (size_t i = 0; i < n; i++) c[i] = (unsigned char)(seed + i * 7 + (i >> 8));
}
static int same(const void *p, size_t n, unsigned seed) {
    const unsigned char *c = p;
    for (size_t i = 0; i < n; i++)
        if (c[i] != (unsigned char)(seed + i * 7 + (i >> 8))) return 0;
    return 1;
}

int main(void) {
    printf("%d\n", my_realloc(NULL, 0) == NULL);
    printf("%d\n", my_realloc(NULL, ((size_t)1 << 30) + 1) == NULL);
    printf("%d\n", my_realloc(NULL, SIZE_MAX) == NULL);
    my_heap_dump();
    printf("--\n");
    unsigned char *a = my_realloc(NULL, 50);
    fill(a, 50, 3);
    unsigned char *b = my_realloc(NULL, 300);
    fill(b, 300, 4);
    my_heap_dump();
    my_set_policy(NEXT_FIT);
    my_free(a);
    unsigned char *c = my_malloc(10);      /* next fit from end of b (0:512) */
    my_set_policy(FIRST_FIT);
    unsigned char *d = my_realloc(NULL, 10);
    printf("d at a %d\n", d == a);
    printf("b data %d\n", same(b, 300, 4));
    my_heap_dump();
    my_leak_report();
    (void)c;
    return 0;
}
