#include "myalloc.h"
#include <stdio.h>

int main(void) {
    /*
       Test BEST_FIT and WORST_FIT tie-breaking (earliest in heap order, including across regions)
       and exact vs non-splitting fit.
       Region 0:
       a = my_malloc(136) -> B = 176 -> 0:32 A 176 136
       x1 = my_malloc(10) -> 0:208 A 128 10
       b = my_malloc(104) -> B = 144 -> 0:336 A 144 104
       x2 = my_malloc(10) -> 0:480 A 128 10
       c = my_malloc(104) -> B = 144 -> 0:608 A 144 104
       x3 = my_malloc(10) -> 0:752 A 128 10
       d = my_malloc(136) -> B = 176 -> 0:880 A 176 136
       x4 = my_malloc(64440) -> B = 32+64440+8 = 64480 -> 0:1056 A 64480 64440 (fills region 0: 1056+64480=65536)

       Region 1:
       e = my_malloc(104) -> B = 144 -> 1:32 A 144 104
       x5 = my_malloc(65320) -> B = 32+65320+8 = 65360 -> 1:176 A 65360 65320 (fills region 1: 176+65360=65536)

       Now free a, b, c, d, e:
       Free blocks are:
       0:32  F 176
       0:336 F 144
       0:608 F 144
       0:880 F 176
       1:32  F 144

       1) Under BEST_FIT, request 88 bytes (B = 128):
          Smallest free blocks >= 128 have size 144 (0:336, 0:608, 1:32).
          Must pick EARLIEST in heap order: 0:336 F 144!
          144 - 128 = 16 < 128, so no split -> 0:336 A 144 88.
       2) Under WORST_FIT, request 88 bytes (B = 128):
          Largest free blocks have size 176 (0:32, 0:880).
          Must pick EARLIEST in heap order (not last!): 0:32 F 176!
          176 - 128 = 48 < 128, so no split -> 0:32 A 176 88.
       3) Under BEST_FIT, request 88 bytes (B = 128) again:
          Remaining size-144 blocks are 0:608 F 144 and 1:32 F 144.
          Must pick 0:608 F 144 (region 0 before region 1, regardless of mmap virtual addresses!).
       4) Under WORST_FIT, request 200 bytes (B = 240):
          Largest free block is 0:880 F 176 < 240, so heap grows with Region 2 (65536)!
    */
    void *a = my_malloc(136);
    void *x1 = my_malloc(10);
    void *b = my_malloc(104);
    void *x2 = my_malloc(10);
    void *c = my_malloc(104);
    void *x3 = my_malloc(10);
    void *d = my_malloc(136);
    void *x4 = my_malloc(64440);
    void *e = my_malloc(104);
    void *x5 = my_malloc(65320);
    (void)x1; (void)x2; (void)x3; (void)x4; (void)x5;

    my_free(a);
    my_free(b);
    my_free(c);
    my_free(d);
    my_free(e);

    my_set_policy(BEST_FIT);
    void *p1 = my_malloc(88);

    my_set_policy(WORST_FIT);
    void *p2 = my_malloc(88);

    my_set_policy(BEST_FIT);
    void *p3 = my_malloc(88);

    my_set_policy(WORST_FIT);
    void *p4 = my_malloc(200);
    (void)p1; (void)p2; (void)p3; (void)p4;

    my_heap_dump();
    return 0;
}
