#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static int allbytes(const void *p, size_t n, unsigned char v) {
    const unsigned char *c = p;
    for (size_t i = 0; i < n; i++)
        if (c[i] != v) return 0;
    return 1;
}

int main(void) {
    unsigned char *a = my_malloc(100);
    unsigned char *b = my_malloc(10);
    unsigned char *c = my_malloc(100);
    unsigned char *d = my_malloc(10);
    unsigned char *e = my_malloc(100);
    unsigned char *f = my_malloc(10);
    memset(a, 0x11, 100);
    memset(c, 0x22, 100);
    memset(e, 0x33, 100);
    my_free(a);
    my_free(c);
    my_free(e);
    /* next-fit position is 0:848 (end of f) */
    my_set_policy(FIRST_FIT);
    unsigned char *x = my_calloc(2, 50);   /* first fit -> 0:32, position -> 0:176 */
    printf("x zero %d\n", allbytes(x, 100, 0));
    my_set_policy(NEXT_FIT);
    unsigned char *y = my_malloc(100);     /* first block ending after 0:176 */
    unsigned char *z = my_calloc(100, 1);  /* next fit, dirty block of e */
    printf("z zero %d\n", allbytes(z, 100, 0));
    unsigned char *w = my_malloc(100);
    my_heap_dump();
    my_free(x);
    printf("bad %d %d\n", my_calloc(0, 5) == NULL, my_calloc(1, ((size_t)1 << 30) + 1) == NULL);
    unsigned char *u = my_malloc(10);      /* position unchanged by failed callocs */
    my_heap_dump();
    (void)b; (void)d; (void)f; (void)y; (void)w; (void)u;
    return 0;
}
