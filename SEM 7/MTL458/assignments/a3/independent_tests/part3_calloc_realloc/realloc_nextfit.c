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
    unsigned char *a = my_malloc(10);
    unsigned char *h = my_malloc(500);
    unsigned char *g = my_malloc(10);
    my_free(h);
    unsigned char *x = my_malloc(500); /* reuses 0:160; position -> 0:704 */
    fill(a, 10, 1);
    fill(x, 500, 2);
    my_set_policy(NEXT_FIT);

    /* shrink in place: the split-off tail ends exactly at the position (0:704) */
    printf("same %d\n", my_realloc(x, 100) == x);
    unsigned char *y = my_malloc(100); /* tail block does not end after 0:704 */
    my_heap_dump();

    /* grow in place: position stays at 0:976 */
    printf("same %d\n", my_realloc(x, 300) == x);
    fill(x, 300, 2);
    unsigned char *z = my_malloc(10);
    my_heap_dump();

    /* a first-fit move sets the position to the end of the new block (0:704) */
    my_free(y);
    my_set_policy(FIRST_FIT);
    unsigned char *a2 = my_realloc(a, 100);
    printf("moved %d data %d\n", a2 != a, same(a2, 10, 1));
    my_set_policy(NEXT_FIT);
    unsigned char *w = my_malloc(10); /* first free block ending after 0:704 */
    my_heap_dump();

    /* next-fit move, then shrink whose tail merges with the free block holding the position */
    unsigned char *x2 = my_realloc(x, 400);
    printf("moved %d data %d\n", x2 != x, same(x2, 300, 2));
    unsigned char *v = my_malloc(300);
    fill(v, 300, 3);
    printf("same %d\n", my_realloc(v, 10) == v);
    unsigned char *u = my_malloc(100);
    my_heap_dump();

    /* next-fit move that has to wrap around */
    unsigned char *big = my_malloc(63000);
    fill(big, 63000, 4);
    fill(w, 10, 5);
    unsigned char *w2 = my_realloc(w, 400);
    printf("moved %d data %d\n", w2 != w, same(w2, 10, 5));
    fill(z, 10, 6);
    unsigned char *z2 = my_realloc(z, 300);
    printf("moved %d data %d\n", z2 != z, same(z2, 10, 6));
    unsigned char *s = my_malloc(10);
    my_heap_dump();
    printf("data %d %d %d\n", same(big, 63000, 4), same(x2, 300, 2), same(v, 10, 3));
    my_leak_report();
    (void)g; (void)u; (void)s;
    return 0;
}
