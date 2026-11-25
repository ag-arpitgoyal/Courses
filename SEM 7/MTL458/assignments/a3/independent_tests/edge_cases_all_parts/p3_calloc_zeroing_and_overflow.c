#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    /* 1. Allocate a block, fill with 0xAA, free it, then my_calloc it and verify all bytes are 0. */
    unsigned char *a = my_malloc(80);
    memset(a, 0xAA, 80);
    my_free(a);

    unsigned char *b = my_calloc(10, 8);
    int all_zero = 1;
    for (int i = 0; i < 80; i++) {
        if (b[i] != 0) all_zero = 0;
    }
    printf("all_zero=%d\n", all_zero);

    /* 2. Test my_calloc boundary cases after region 0 exists:
       - nmemb = 0 or size = 0 -> NULL
       - nmemb * size > 2^30 -> NULL
       - nmemb * size overflows size_t (e.g. (1ULL<<32) * (1ULL<<32) == 0 mod 2^64, or (1ULL<<33)+1 * (1ULL<<31) == (1ULL<<31) <= 2^30 mod 2^64!) -> NULL
       Notice: ((1ULL << 34) + 1) * (1ULL << 30) wraps around mod 2^64 to (1ULL << 30) = 2^30!
       A naive check `(nmemb * size) > (1ULL << 30)` without overflow check would PASS 2^30 and allocate!
       Even smaller: ((1ULL << 50) + 1) * (1ULL << 14) wraps around mod 2^64 to 16384! */
    void *c1 = my_calloc(0, 50);
    void *c2 = my_calloc(50, 0);
    void *c3 = my_calloc(1ULL << 20, (1ULL << 10) + 1);
    void *c4 = my_calloc((1ULL << 50) + 1, 1ULL << 14);
    printf("nulls=%d\n", (c1 == NULL && c2 == NULL && c3 == NULL && c4 == NULL));

    my_heap_dump();
    return 0;
}
