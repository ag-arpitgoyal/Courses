/* Buddy: fill the whole 1 MiB region with 32768 32-byte blocks, check the next
 * request fails, then free them all in "evenodd" order with intermediate dumps.
 * Expected output from model/buddy.py (and checked against hand formulas). */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char *P[32768];

static int bitrev15(int j)
{
    int r = 0;
    for (int b = 0; b < 15; b++)
        if (j & (1 << b))
            r |= 1 << (14 - b);
    return r;
}

int main(void)
{
    (void)bitrev15;
    my_set_policy(BUDDY);
    for (int i = 0; i < 32768; i++) {
        P[i] = my_malloc((size_t)(1 + i % 16));
        if (!P[i])
            printf("NULL at %d\n", i);
        else
            P[i][0] = (unsigned char)i;
    }
    my_heap_dump();
    printf("extra %s\n", my_malloc(1) ? "ptr" : "NULL");
    for (int j = 0; j < 32768; j++) {
        int i = j < 16384 ? 2 * j : 2 * (j - 16384) + 1;
        my_free(P[i]);
        if (j == 8191 || j == 16383 || j == 24575)
            my_heap_dump();
    }
    my_heap_dump();
    return 0;
}
