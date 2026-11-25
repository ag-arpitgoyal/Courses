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

static int k;
static void R(void *p) {
    printf("calloc %d -> %s\n", k++, p == NULL ? "NULL" : "non-NULL");
}

static void bad_callocs(void) {
    R(my_calloc(0, 5));
    R(my_calloc(5, 0));
    R(my_calloc(0, 0));
    R(my_calloc(0, SIZE_MAX));
    R(my_calloc(1, ((size_t)1 << 30) + 1));
    R(my_calloc(((size_t)1 << 30) + 1, 1));
    R(my_calloc(((size_t)1 << 15) + 1, (size_t)1 << 15));
    R(my_calloc(2, ((size_t)1 << 29) + 1));
    R(my_calloc((size_t)1 << 31, 1));
    R(my_calloc(SIZE_MAX, 1));
    /* products that overflow size_t (wrapped values in comments) */
    R(my_calloc(SIZE_MAX, SIZE_MAX));             /* wraps to 1 */
    R(my_calloc(((size_t)1 << 63) + 1, 2));       /* wraps to 2 */
    R(my_calloc((size_t)1 << 32, (size_t)1 << 32)); /* wraps to 0 */
    R(my_calloc(SIZE_MAX, 2));                    /* wraps to SIZE_MAX - 1 */
    R(my_calloc(((size_t)1 << 62) + 1, 4));       /* wraps to 4 */
    R(my_calloc(2, ((size_t)1 << 63) + 8));       /* wraps to 16 */
    R(my_calloc(SIZE_MAX / 3 + 1, 3));            /* wraps to 2 */
    R(my_calloc(((size_t)1 << 33) + 1, (size_t)1 << 31)); /* wraps to 2^31 */
}

int main(void) {
    bad_callocs();
    my_heap_dump();
    printf("--\n");
    unsigned char *p = my_calloc(1, 1);
    printf("p %d zero %d\n", p != NULL, p && allbytes(p, 1, 0));
    k = 0;
    bad_callocs();
    my_heap_dump();
    unsigned char *q = my_calloc(1000, 1000);
    printf("q %d zero %d\n", q != NULL, q && allbytes(q, 1000000, 0));
    my_heap_dump();
    my_leak_report();
    return 0;
}
