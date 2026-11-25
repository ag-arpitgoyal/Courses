#include "myalloc.h"
#include <stdio.h>

int main(void) {
    /*
       Test that when my_realloc moves a block while delay > 0:
       1. It checks guard bytes of the old block first.
       2. It places a new block for n bytes using the current policy.
       3. It frees the old block as my_free would (putting it into the delay queue with 0xDE,
          and evicting the front of the delay queue if queue size > k).
       Also test my_leak_report():
       - Reports only ALLOCATED blocks in heap order across multiple regions.
       - Sums the REQUESTED sizes (not block sizes).
       - Does NOT report free or delayed blocks.
    */
    my_set_delay(1);
    unsigned char *a = my_malloc(10);      /* 0:32 A 128 10 */
    unsigned char *b = my_malloc(20);      /* 0:160 A 128 20 */
    void *blocker = my_malloc(30);         /* 0:288 A 128 30 */
    void *c = my_malloc(65500);            /* 1:32 A 65552 65500 (Region 1, 131072) */
    (void)blocker; (void)c;

    /* Free a -> enters delay queue [a] (0:32 D 128 10). Write to a[5] to trigger write-after-free when evicted! */
    my_free(a);
    a[5] = 0x99;

    /* Corrupt guard byte of b (b[20]) and realloc b to 100 bytes (B=144).
       Since blocker (0:288) is allocated, b MUST MOVE to 0:416 A 144 100!
       1. Guard check on b prints: error: overflow detected (block 0:160)
       2. New block allocated at 0:416 A 144 100
       3. Old block b (0:160) enters delay queue -> evicts a (0:32) -> prints: error: write after free (block 0:32)
       4. Queue now holds [b] (0:160 D 128 20), and 0:32 is now F 128! */
    b[20] = 0xEE;
    b = my_realloc(b, 100);
    (void)b;

    my_heap_dump();
    my_leak_report();
    return 0;
}
