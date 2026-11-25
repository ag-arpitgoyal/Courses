#include "myalloc.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    /*
       Test shrink in place:
       - when S - B == 112 (< 128): does NOT split, updates requested size to new n, keeps S.
       - when S - B == 128 (>= 128): splits off 128-byte free block and coalesces with next block if free!
       Setup:
       a = my_malloc(216) -> 0:32 A 256 216
       b = my_malloc(10)  -> 0:288 A 128 10
       c = my_malloc(10)  -> 0:416 A 128 10
    */
    unsigned char *a = my_malloc(216);
    void *b = my_malloc(10);
    void *c = my_malloc(10);
    (void)c;
    for (int i = 0; i < 20; i++) a[i] = (unsigned char)(i + 1);

    /* Shrink a to 104 bytes -> B = 144. S - B = 256 - 144 = 112 < 128 -> no split!
       Block at 0:32 remains size 256, requested becomes 104. */
    unsigned char *a2 = my_realloc(a, 104);
    printf("same_ptr1=%d\n", a2 == a);
    my_heap_dump();

    /* Free b (0:288 F 128). Now shrink a2 to 88 bytes -> B = 128.
       S - B = 256 - 128 = 128 >= 128 -> splits off 0:160 F 128, which coalesces with 0:288 F 128 into 0:160 F 256! */
    my_free(b);
    unsigned char *a3 = my_realloc(a2, 88);
    printf("same_ptr2=%d\n", a3 == a);
    int ok = 1;
    for (int i = 0; i < 20; i++) {
        if (a3[i] != (unsigned char)(i + 1)) ok = 0;
    }
    printf("data_ok=%d\n", ok);
    my_heap_dump();
    return 0;
}
