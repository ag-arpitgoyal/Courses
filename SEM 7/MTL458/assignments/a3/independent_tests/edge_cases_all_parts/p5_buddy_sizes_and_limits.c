#include "myalloc.h"
#include <stdio.h>

int main(void) {
    /*
       Test buddy allocator boundary conditions:
       1. Before any valid allocation, my_heap_dump() prints nothing, and my_malloc(0) or my_malloc(1048561)
          returns NULL without creating the buddy region!
       2. Exact full-region allocation: n = 1048560 -> n + 16 = 1048576 = 2^20 -> allocates the entire region at offset 0!
          While full, my_malloc(1) returns NULL (never grows).
       3. Freeing the full block merges back to 0 F 1048576.
       4. Block size power-of-2 boundaries:
          - n = 1  -> max(32, next_pow2(17)) = 32
          - n = 16 -> max(32, next_pow2(32)) = 32
          - n = 17 -> max(32, next_pow2(33)) = 64
          - n = 48 -> max(32, next_pow2(64)) = 64
          - n = 49 -> max(32, next_pow2(65)) = 128
    */
    my_set_policy(BUDDY);
    my_heap_dump();
    void *bad0 = my_malloc(0);
    void *bad1 = my_malloc(1048561);
    printf("initial_nulls=%d\n", (bad0 == NULL && bad1 == NULL));
    my_heap_dump();

    void *full = my_malloc(1048560);
    void *oom = my_malloc(1);
    printf("full_ok=%d oom_null=%d\n", full != NULL, oom == NULL);
    my_heap_dump();

    my_free(full);

    void *p1 = my_malloc(1);
    void *p16 = my_malloc(16);
    void *p17 = my_malloc(17);
    void *p48 = my_malloc(48);
    void *p49 = my_malloc(49);
    (void)p1; (void)p16; (void)p17; (void)p48; (void)p49;
    my_heap_dump();
    return 0;
}
