#include "myalloc.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    /*
       Test grow in place vs move:
       1. Grow in place when S + next_free - B == 112 (< 128): absorbs entire next free block (no split).
       2. Grow in place when S + next_free - B == 128 (>= 128): splits off 128-byte free block.
       3. Previous block is free and large enough, next block is allocated: MUST NOT use previous block;
          must Move instead, and MUST NOT reuse the old block itself during the Move!
    */
    unsigned char *a = my_malloc(88);  /* 0:32 A 128 88 */
    void *b = my_malloc(88);           /* 0:160 A 128 88 */
    void *c = my_malloc(10);           /* 0:288 A 128 10 */
    unsigned char *d = my_malloc(88);  /* 0:416 A 128 88 */
    void *e = my_malloc(216);          /* 0:544 A 256 216 */
    void *f = my_malloc(10);           /* 0:800 A 128 10 */
    (void)c; (void)f;

    for (int i = 0; i < 88; i++) {
        a[i] = (unsigned char)(i ^ 0x55);
        d[i] = (unsigned char)(i ^ 0x33);
    }

    /* Case 1: free b (0:160 F 128). Grow a to 104 bytes (B = 144).
       S + next = 128 + 128 = 256 >= 144. Leftover = 256 - 144 = 112 < 128 -> no split!
       a becomes 0:32 A 256 104. */
    my_free(b);
    unsigned char *a2 = my_realloc(a, 104);
    printf("a_in_place=%d\n", a2 == a);

    /* Case 2: free e (0:544 F 256). Grow d (0:416 A 128 88) to 216 bytes (B = 256).
       S + next = 128 + 256 = 384 >= 256. Leftover = 384 - 256 = 128 >= 128 -> splits!
       d becomes 0:416 A 256 216, followed by 0:672 F 128. */
    my_free(e);
    unsigned char *d2 = my_realloc(d, 216);
    printf("d_in_place=%d\n", d2 == d);
    my_heap_dump();

    /* Case 3: Now free a2 (0:32 F 256). Next block is c (0:288 A 128 10), whose PREVIOUS block is 0:32 F 256
       and NEXT block is d2 (0:416 A 256 216, allocated!).
       If we grow c to 150 bytes (B = 208):
       The block BEFORE c (0:32 F 256) is never used to grow in place!
       Instead c must MOVE, and under FIRST_FIT it is placed at 0:32 F 256 (splitting into 0:32 A 208 150? Wait: 256 - 208 = 48 < 128, so 0:32 A 256 150),
       and old c (0:288) is freed! */
    my_free(a2);
    void *c2 = my_realloc(c, 150);
    printf("c_moved=%d\n", c2 != c);

    int d_ok = 1;
    for (int i = 0; i < 88; i++) {
        if (d2[i] != (unsigned char)(i ^ 0x33)) d_ok = 0;
    }
    printf("d_data_ok=%d\n", d_ok);
    my_heap_dump();
    return 0;
}
