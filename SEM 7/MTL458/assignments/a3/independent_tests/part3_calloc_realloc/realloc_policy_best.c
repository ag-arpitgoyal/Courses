#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

#define POL BEST_FIT

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
    unsigned char *a = my_malloc(248);
    unsigned char *x1 = my_malloc(10);
    unsigned char *b = my_malloc(120);
    unsigned char *x2 = my_malloc(10);
    unsigned char *c = my_malloc(360);
    unsigned char *x3 = my_malloc(10);
    unsigned char *m = my_malloc(10);
    unsigned char *x4 = my_malloc(10);
    my_free(b);
    unsigned char *y = my_malloc(120); /* next-fit position -> 0:608 */
    my_free(y);
    my_free(a);
    my_free(c);
    fill(m, 10, 4);
    my_heap_dump();
    my_set_policy(POL);
    unsigned char *q = my_realloc(m, 100); /* B = 144: must move */
    printf("moved %d data %d\n", q != m, same(q, 10, 4));
    fill(q, 100, 4);
    my_heap_dump();
    my_set_policy(NEXT_FIT);
    unsigned char *z = my_malloc(10); /* shows where the move left the next-fit position */
    my_heap_dump();
    my_set_policy(POL);
    unsigned char *r = my_realloc(q, 70000); /* nothing fits: new region */
    printf("moved %d data %d\n", r != q, same(r, 100, 4));
    my_set_policy(NEXT_FIT);
    unsigned char *w = my_malloc(10);
    my_heap_dump();
    my_leak_report();
    (void)x1; (void)x2; (void)x3; (void)x4; (void)z; (void)w;
    return 0;
}
