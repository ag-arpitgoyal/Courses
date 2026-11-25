#include "myalloc.h"
#include <stdio.h>

int main(void) {
    /*
       Test my_realloc special cases:
       1. my_realloc(NULL, n) -> behaves as my_malloc(n)
       2. my_realloc(ptr, (1ULL<<30) + 1) -> returns NULL, leaves block unchanged, AND checks guard bytes (Clarification 3)!
       3. my_realloc(ptr, 0) -> checks guard bytes, frees block (or delays if delay > 0), returns NULL.
    */
    unsigned char *a = my_realloc(NULL, 10); /* 0:32 A 128 10 */
    unsigned char *b = my_realloc(NULL, 20); /* 0:160 A 128 20 */

    /* Huge realloc without overflow: returns NULL, block unchanged */
    void *r1 = my_realloc(a, (1ULL << 30) + 1);
    printf("r1_null=%d\n", r1 == NULL);

    /* Corrupt guard byte of a (at a[10]) and call huge realloc -> must print overflow error AND leave block unchanged! */
    a[10] = 0x55;
    void *r2 = my_realloc(a, (1ULL << 30) + 100);
    printf("r2_null=%d\n", r2 == NULL);
    my_heap_dump();

    /* Fix guard byte of a back to 0xCA, corrupt guard byte of b (at b[20]), and call my_realloc(b, 0) with delay=1 */
    a[10] = 0xCA;
    b[20] = 0x77;
    my_set_delay(1);
    void *r3 = my_realloc(b, 0);
    printf("r3_null=%d\n", r3 == NULL);
    my_heap_dump();
    return 0;
}
