#include "myalloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define N_REG 500
#define PAIRS_PER_REG 200

static unsigned char *holes[N_REG][PAIRS_PER_REG];
static unsigned char *pins[N_REG][PAIRS_PER_REG];
static unsigned char *window[200];
static unsigned char *kept[N_REG];

static size_t hole_req(int k) {
    if (k == 199) return 400;
    switch (k & 3) {
        case 0: return 88;
        case 1: return 104;
        case 2: return 120;
        default: return 136;
    }
}

static size_t pin_req(int k) {
    return (k == 199) ? 9320 : 10;
}

int main(void) {
    my_set_policy(FIRST_FIT);

    /* Phase 1: Allocate 200,000 blocks across 500 regions, then free 100,000 holes */
    for (int r = 0; r < N_REG; r++) {
        for (int k = 0; k < PAIRS_PER_REG; k++) {
            holes[r][k] = my_malloc(hole_req(k));
            pins[r][k] = my_malloc(pin_req(k));
            if (!holes[r][k] || !pins[r][k]) {
                printf("FAIL phase 1 alloc %d %d\n", r, k);
                return 1;
            }
            pins[r][k][0] = (unsigned char)((r + k) & 0xFF);
        }
    }

    for (int r = 0; r < N_REG; r++) {
        for (int k = 0; k < PAIRS_PER_REG; k++) {
            my_free(holes[r][k]);
            holes[r][k] = NULL;
        }
    }

    /* Heap now has 100,000 free blocks.
     * Phase 2: Perform > 120,000 calls under BEST_FIT while ~100,000 free blocks remain active. */
    my_set_policy(BEST_FIT);
    static const size_t reqs[5] = { 80, 100, 115, 130, 380 };
    for (int i = 0; i < 60000; i++) {
        int slot = i % 200;
        if (window[slot] != NULL) {
            my_free(window[slot]);
            window[slot] = NULL;
        }
        size_t req = reqs[i % 5];
        window[slot] = my_malloc(req);
        if (!window[slot]) {
            printf("FAIL phase 2 malloc %d\n", i);
            return 1;
        }
        window[slot][0] = (unsigned char)(i & 0xFF);
    }
    for (int slot = 0; slot < 200; slot++) {
        if (window[slot] != NULL) {
            my_free(window[slot]);
            window[slot] = NULL;
        }
    }

    /* Allocate 500 BEST_FIT blocks of size 380 (B=432), which must each select the 448-byte hole in region r */
    for (int r = 0; r < N_REG; r++) {
        kept[r] = my_malloc(380);
        if (!kept[r]) {
            printf("FAIL kept malloc %d\n", r);
            return 1;
        }
    }

    /* Phase 3: Verify pin data integrity and coalesce pins 1..199 */
    for (int r = 0; r < N_REG; r++) {
        if (pins[r][0][0] != (unsigned char)(r & 0xFF)) {
            printf("CORRUPT pin 0 in region %d\n", r);
        }
        for (int k = 1; k < PAIRS_PER_REG; k++) {
            if (pins[r][k][0] != (unsigned char)((r + k) & 0xFF)) {
                printf("CORRUPT pin %d in region %d\n", k, r);
            }
            my_free(pins[r][k]);
            pins[r][k] = NULL;
        }
    }

    my_heap_dump();
    my_leak_report();
    return 0;
}
