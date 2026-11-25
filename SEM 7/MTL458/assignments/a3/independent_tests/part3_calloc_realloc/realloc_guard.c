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
    unsigned char *a = my_malloc(100);
    unsigned char *b = my_malloc(10);
    unsigned char *c = my_malloc(10);
    unsigned char *e = my_malloc(10);
    fill(a, 100, 1);
    fill(c, 10, 3);

    /* same size: guard checked, then rewritten */
    a[107] = 0;
    printf("1 same %d\n", my_realloc(a, 100) == a);
    printf("2 same %d\n", my_realloc(a, 100) == a);

    /* shrink without split: guard follows the new n */
    printf("3 same %d\n", my_realloc(a, 50) == a);
    a[50] = 0;
    printf("4 same %d\n", my_realloc(a, 60) == a);
    a[63] = 0;
    printf("5 same %d\n", my_realloc(a, 40) == a);
    printf("data %d\n", same(a, 40, 1));
    /* growing inside the block: the old guard bytes become data */
    printf("6 same %d\n", my_realloc(a, 90) == a);
    memset(a + 40, 0x33, 50);
    printf("7 same %d\n", my_realloc(a, 90) == a);
    my_heap_dump();

    /* grow in place (absorbing a 32-byte leftover) */
    my_free(b);
    printf("8 same %d\n", my_realloc(a, 200) == a);
    a[205] = 1;
    printf("9 same %d\n", my_realloc(a, 200) == a);
    my_heap_dump();

    /* move: error reported once, for the old block */
    c[17] = 0;
    unsigned char *q = my_realloc(c, 500);
    printf("10 moved %d data %d\n", q != c, same(q, 10, 3));
    my_heap_dump();
    q[500] = 0;
    my_free(q);
    my_heap_dump();

    /* n == 0 */
    a[207] = 9;
    printf("11 null %d\n", my_realloc(a, 0) == NULL);
    my_heap_dump();

    /* shrink with split */
    unsigned char *d = my_malloc(1000);
    printf("12 same %d\n", my_realloc(d, 100) == d);
    my_heap_dump();
    d[101] = 0;
    my_free(d);
    my_heap_dump();
    (void)e;
    return 0;
}
