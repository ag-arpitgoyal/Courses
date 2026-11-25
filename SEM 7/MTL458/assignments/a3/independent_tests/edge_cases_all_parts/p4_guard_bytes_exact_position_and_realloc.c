#include "myalloc.h"
#include <stdio.h>

int main(void) {
    /*
       Test guard bytes position and updates across my_realloc:
       1. Guard bytes are the 8 bytes IMMEDIATELY after the user's n bytes (indices n .. n+7),
          NOT the last 8 bytes of the block!
       2. Check that all 8 guard bytes start as 0xCA.
       3. Corrupt multiple guard bytes -> prints "error: overflow detected (block R:O)" AT MOST ONCE per call,
          and still carries on with the call as normal.
       4. After my_realloc (shrink in place, grow in place, move), the new 8 guard bytes follow the NEW n bytes.
    */
    unsigned char *a = my_malloc(10); /* 0:32 A 128 10, guard bytes at a[10..17] */
    int guard_init_ok = 1;
    for (int i = 10; i < 18; i++) {
        if (a[i] != 0xCA) guard_init_ok = 0;
    }
    printf("guard_init_ok=%d\n", guard_init_ok);

    /* Corrupt BOTH a[10] and a[17] (first and last guard byte) and realloc in place to 25 bytes (still B=128).
       Must print overflow detected ONCE, and set new guard bytes at a[25..32] to 0xCA! */
    a[10] = 0x00;
    a[17] = 0xFF;
    a = my_realloc(a, 25);

    int guard_grow_ok = 1;
    for (int i = 25; i < 33; i++) {
        if (a[i] != 0xCA) guard_grow_ok = 0;
    }
    printf("guard_grow_ok=%d\n", guard_grow_ok);

    /* Now shrink a in place to 12 bytes. Since a[25..32] are untouched 0xCA, no overflow is reported!
       And new guard bytes are placed at a[12..19] (which were user bytes during n=25!).
       Let's dirty a[12..19] BEFORE shrinking to 12 bytes to prove my_realloc resets a[12..19] to 0xCA! */
    for (int i = 0; i < 25; i++) a[i] = 0x11;
    a = my_realloc(a, 12);

    int guard_shrink_ok = 1;
    for (int i = 12; i < 20; i++) {
        if (a[i] != 0xCA) guard_shrink_ok = 0;
    }
    printf("guard_shrink_ok=%d\n", guard_shrink_ok);

    /* Now corrupt the 8th guard byte a[19] (index n+7) and free a: must detect overflow at 0:32 and free it! */
    a[19] = 0xCB;
    my_free(a);
    my_heap_dump();
    return 0;
}
