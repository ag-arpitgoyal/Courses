#include "myalloc.h"
#include <stdio.h>

int main(void) {
    /* Under FIRST_FIT initially:
       a: 0:32 A 256 216
       b: 0:288 A 128 10
       c: 0:416 A 128 10
       d: 0:544 A 128 10
       e: 0:672 A 128 10
       Free a, b, d -> 0:32 F 384, 0:416 A 128 10, 0:544 F 128, 0:672 A 128 10, 0:800 F 64736.
       Now allocate x = my_malloc(10) under FIRST_FIT:
       Places at 0:32 A 128 10, splits off 0:160 F 256.
       Next-fit position moves to end of x = 0:160 (even under FIRST_FIT!).
       Case 1: 0:160 F 256 starts AT 0:160 and ends at 0:416 > 0:160.
       Switch to NEXT_FIT and allocate y = my_malloc(10):
       Must pick 0:160 F 256 -> becomes 0:160 A 128 10, 0:288 F 128.
       Next-fit position is now 0:288.
       Case 2: Free x (0:32) and y (0:160) -> coalesces with 0:288 F 128 into 0:32 F 384!
       Now next-fit position 0:288 is strictly INSIDE 0:32 F 384 (starts at 32 < 288, ends at 416 > 288).
       By Clarification 6, 0:32 F 384 ends after 0:288, so z = my_malloc(10) under NEXT_FIT MUST choose 0:32 F 384!
       z becomes 0:32 A 128 10, 0:160 F 256, and next-fit pos becomes 0:160.
    */
    void *a = my_malloc(216);
    void *b = my_malloc(10);
    void *c = my_malloc(10);
    void *d = my_malloc(10);
    void *e = my_malloc(10);
    (void)c; (void)e;

    my_free(a);
    my_free(b);
    my_free(d);

    void *x = my_malloc(10);
    my_set_policy(NEXT_FIT);
    void *y = my_malloc(10);
    my_heap_dump();

    my_free(x);
    my_free(y);
    void *z = my_malloc(10);
    (void)z;
    my_heap_dump();
    return 0;
}
