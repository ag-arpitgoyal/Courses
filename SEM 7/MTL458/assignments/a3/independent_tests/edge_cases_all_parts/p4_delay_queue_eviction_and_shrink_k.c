#include "myalloc.h"
#include <stdio.h>

int main(void) {
    /*
       Test delay queue mechanics:
       1. Delayed blocks are NOT coalesced with adjacent free or delayed blocks, and are NOT reused by allocations.
       2. Freeing or reallocating a delayed block prints "error: double free (block R:O)" and does NOT remove or duplicate it in the queue.
       3. When evicted from the delay queue, ONLY the first n bytes are checked for 0xDE (not guard bytes or padding),
          and at most one "error: write after free" message is printed per evicted block even if multiple bytes were modified.
       4. Lowering k with my_set_delay(k) evicts from the front in FIFO order until queue size <= k,
          checking 0xDE and coalescing each evicted block as normal.
       5. Realloc (Move) and Realloc (n=0) both go through the delay queue when k > 0!
    */
    my_set_delay(3);
    unsigned char *a = my_malloc(10); /* 0:32 A 128 10 */
    unsigned char *b = my_malloc(15); /* 0:160 A 128 15 */
    unsigned char *c = my_malloc(20); /* 0:288 A 128 20 */
    unsigned char *d = my_malloc(25); /* 0:416 A 128 25 */

    /* Free a, b, c -> all 3 enter delay queue [a, b, c]. None coalesce! */
    my_free(a);
    my_free(b);
    my_free(c);
    my_heap_dump();

    /* Double free on delayed block b -> prints error, leaves queue [a, b, c] untouched */
    my_free(b);
    void *r = my_realloc(b, 30);
    (void)r;

    /* Modify multiple bytes of a (a[0] and a[9]) and 1 last byte of c (c[19]). Leave b clean. */
    a[0] = 0x01;
    a[9] = 0x02;
    c[19] = 0x03;

    /* Now call my_realloc(d, 0) -> frees d via delay queue!
       Queue becomes [a, b, c, d] (size 4 > 3) -> evicts front block 'a' (0:32)!
       Prints "error: write after free (block 0:32)" ONCE, and frees 0:32 F 128 (cannot coalesce with 0:160 D 128 15!). */
    my_realloc(d, 0);
    my_heap_dump();

    /* Allocate e = my_malloc(10) -> reuses 0:32 F 128, skips delayed blocks b, c, d! */
    void *e = my_malloc(10);
    (void)e;

    /* Now lower delay to 1: evicts 'b' (0:160, clean -> becomes 0:160 F 128),
       then evicts 'c' (0:288, dirty at c[19] -> prints write after free for 0:288, then coalesces with 0:160 F 128 into 0:160 F 256!).
       Queue now has [d] (size 1). */
    my_set_delay(1);
    my_heap_dump();

    /* Lower delay to 0: evicts 'd' (0:416, clean -> coalesces with 0:160 F 256 AND 0:544 F 64992 into 0:160 F 65376!). */
    my_set_delay(0);
    my_heap_dump();
    return 0;
}
