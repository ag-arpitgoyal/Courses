#include "myalloc.h"
#include <stdio.h>

int main(void) {
    /*
       Test buddy allocator placement and merging rules:
       1. Smallest free block of size >= s is chosen EVEN IF a larger free block has a LOWER offset!
          Setup:
          a = my_malloc(16) -> 0 A 32 16
          b = my_malloc(16) -> 32 A 32 16
          c = my_malloc(48) -> 64 A 64 48
          d = my_malloc(48) -> 128 A 64 48
          e = my_malloc(48) -> 192 A 64 48
          Now free a (0) and b (32) -> they are buddies (0 XOR 32 = 32), so they merge into 0 F 64!
          Wait: if 0 F 64 and c (64 A 64 48) are at 0 and 64, let's also free c (64) -> merges with 0 F 64 into 0 F 128!
          Now free e (192 A 64 48) -> its buddy is d (128 A 64 48, still allocated!), so 192 F 64 stays size 64!
          Now we have:
            0   F 128
            128 A 64 48
            192 F 64
            256 F 256 ...
          Request f = my_malloc(48) (needs s = 64):
          Smallest free block >= 64 is 192 F 64 (size 64), whereas 0 F 128 has a lower offset (0) but larger size (128)!
          Buddy rule: "Take the smallest free block of size at least s, the one with the lowest offset if there are several."
          So f MUST be placed at 192 (NOT 0)!
       2. Non-buddy adjacent free blocks MUST NOT merge!
          Suppose we instead have:
          0   A 32 16
          32  F 32  (buddy is 0, allocated)
          64  F 32  (buddy is 96, allocated)
          96  A 32 16
          Offsets 32 and 64 are adjacent and both free of size 32, but 32 XOR 32 = 0 != 64!
          They are NOT buddies, so they MUST NOT merge into a 64-byte block at offset 32!
          And when two blocks of size 32 are free at 32 and 64, requesting size 32 picks the lowest offset (32).
    */
    my_set_policy(BUDDY);
    void *a = my_malloc(16); /* 0 A 32 16 */
    void *b = my_malloc(16); /* 32 A 32 16 */
    void *c = my_malloc(48); /* 64 A 64 48 */
    void *d = my_malloc(48); /* 128 A 64 48 */
    void *e = my_malloc(48); /* 192 A 64 48 */

    my_free(a);
    my_free(b);
    my_free(c); /* merges with 0 F 64 -> 0 F 128 */
    my_free(e); /* 192 F 64 (buddy 128 is allocated) */

    void *f = my_malloc(48); /* must pick 192 F 64 (smallest size >= 64), NOT 0 F 128! */
    my_heap_dump();

    /* Now split 0 F 128 into four 32-byte blocks at 0, 32, 64, 96 */
    void *x0 = my_malloc(16); /* 0 A 32 16 */
    void *x1 = my_malloc(16); /* 32 A 32 16 */
    void *x2 = my_malloc(16); /* 64 A 32 16 */
    void *x3 = my_malloc(16); /* 96 A 32 16 */

    /* Free x1 (32) and x2 (64): adjacent size-32 free blocks, but NOT buddies! */
    my_free(x1);
    my_free(x2);
    my_heap_dump();

    /* Now free x0 (0) and x3 (96), d (128), f (192) -> everything cascades and merges back to 0 F 1048576! */
    my_free(x0);
    my_free(x3);
    my_free(d);
    my_free(f);
    my_heap_dump();
    return 0;
}
