#include "myalloc.h"
#include <stdio.h>

int main(void) {
    /*
       Test buddy allocator deep splitting (always keeping lower half, leaving upper half free),
       choosing lowest offset among equal-sized smallest free blocks, and multi-level cascade merging.
       1. Allocate a = my_malloc(100) (s = 128) -> at 0 A 128 100.
       2. Allocate b = my_malloc(100) (s = 128) -> at 128 A 128 100.
       3. Allocate c = my_malloc(100) (s = 128) -> splits 256 F 256 into 256 A 128 100 and 384 F 128.
       4. Allocate d = my_malloc(100) (s = 128) -> uses 384 F 128 -> 384 A 128 100.
       5. Free a (0) and c (256): now both 0 F 128 and 256 F 128 are free of size 128!
          Request e = my_malloc(40) (s = 64):
          Smallest free blocks >= 64 have size 128 (at offsets 0 and 256).
          Tie-breaker: lowest offset (0)!
          Splits 0 F 128 into 0 A 64 40 (lower half kept!) and 64 F 64 (upper half left free!).
    */
    my_set_policy(BUDDY);
    void *a = my_malloc(100);
    void *b = my_malloc(100);
    void *c = my_malloc(100);
    void *d = my_malloc(100);
    (void)b; (void)d;

    my_free(a);
    my_free(c);

    void *e = my_malloc(40);
    (void)e;
    my_heap_dump();
    return 0;
}
