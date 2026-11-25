/* Edge cases, one per command-line name. Each targets one rule of the spec.
 * The expected output for case X is in expected/edge/X.txt.
 *
 * Helpers print "NULL" / "ok" for returned pointers and "data ok" / "data BAD"
 * after checking contents, so the expected files do not depend on addresses. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "myalloc.h"

#define R(expr) printf("%s -> %s\n", #expr, (expr) ? "ok" : "NULL")

static int global_var;

static void fill(void *p, size_t n, int seed) {
    for (size_t i = 0; i < n; i++) ((unsigned char *)p)[i] = (unsigned char)(seed + i * 7);
}
static void check(const char *what, void *p, size_t n, int seed) {
    for (size_t i = 0; i < n; i++)
        if (((unsigned char *)p)[i] != (unsigned char)(seed + i * 7)) { printf("%s: data BAD at %zu\n", what, i); return; }
    printf("%s: data ok\n", what);
}

/* ---------------- Part 1: core rules ---------------- */

/* Dump before any request prints nothing; my_free(NULL) does nothing. */
static void empty_heap(void) {
    my_heap_dump();
    my_free(NULL);
    my_leak_report();
}

/* NULL for n = 0 and n > 2^30 (after the heap exists). */
static void malloc_limits(void) {
    void *a = my_malloc(1);
    R(my_malloc(0));
    R(my_malloc((1UL << 30) + 1));
    R(my_malloc(SIZE_MAX));
    my_free(a);
    my_heap_dump();
}

/* block_size(n) = max(128, round_up(32 + n + 8, 16)). */
static void block_sizes(void) {
    my_malloc(1);      /* 128 */
    my_malloc(88);     /* 32+88+8 = 128 exactly */
    my_malloc(89);     /* 129 -> 144 */
    my_malloc(104);    /* 144 exactly */
    my_malloc(105);    /* 145 -> 160 */
    my_malloc(1000);   /* 1040 */
    my_heap_dump();
}

/* Leftover of 112 (< 128) is not split off; leftover of exactly 128 is. */
static void split_threshold(void) {
    void *a = my_malloc(200);          /* 0:32 size 240 */
    void *s1 = my_malloc(10);          /* barrier 0:272 */
    void *b = my_malloc(216);          /* 0:400 size 256 */
    void *s2 = my_malloc(10);          /* barrier 0:656 */
    (void)s1; (void)s2;
    my_free(a);
    my_free(b);
    my_malloc(88);                     /* B = 128 into 240: leftover 112 -> whole 240 */
    my_malloc(88);                     /* B = 128 into 256: leftover 128 -> split */
    my_heap_dump();
}

/* Freeing a block between two free blocks merges all three. */
static void coalesce_both_sides(void) {
    void *a = my_malloc(10), *b = my_malloc(10), *c = my_malloc(10), *d = my_malloc(10);
    (void)d;
    my_free(a);
    my_free(c);
    my_heap_dump();
    my_free(b);          /* joins a and c */
    my_heap_dump();
}

/* Last block of a region merges with the region's free tail; regions never merge. */
static void regions_never_merge(void) {
    void *a = my_malloc(65464);        /* block 65504 = all of region 0 */
    void *b = my_malloc(1);            /* region 1 */
    my_heap_dump();
    my_free(a);
    my_free(b);
    my_heap_dump();
}

/* Growth: new region size is the smallest multiple of 65536 >= B + 32. */
static void growth_sizes(void) {
    my_malloc(10);                     /* region 0 */
    my_malloc(65456);                  /* B = 65504, B+32 = 65536 -> region 1 of 65536, no free tail */
    my_malloc(65465);                  /* B = 65520, B+32 = 65552 -> region 2 of 131072 */
    my_malloc(131000);                 /* B = 131040, B+32 = 131072 -> region 3 of 131072 */
    my_heap_dump();
}

/* An exact-fit request takes a free block with nothing left over. */
static void exact_fit(void) {
    void *a = my_malloc(100);          /* 144 */
    void *s = my_malloc(10);
    (void)s;
    my_free(a);
    my_malloc(104);                    /* also 144: exact fit at 0:32 */
    my_heap_dump();
}

/* ---------------- Part 2: policies ---------------- */

/* Next fit wraps to the start of the heap when nothing after the position fits. */
static void next_fit_wraps(void) {
    my_set_policy(NEXT_FIT);
    void *a = my_malloc(100);          /* 0:32 */
    my_malloc(100);                    /* 0:176 */
    my_free(a);                        /* hole at 0:32, position is 0:320 */
    my_malloc(65064);                  /* B = 65104 into the 65216 tail: leftover 112, not
                                          split, so the block runs to the region's end */
    my_malloc(100);                    /* nothing ends after the position: wraps to 0:32 */
    my_heap_dump();
}

/* Next fit continues from the last allocation, skipping earlier holes. */
static void next_fit_continues(void) {
    void *a = my_malloc(10), *s1 = my_malloc(10), *b = my_malloc(10), *s2 = my_malloc(10);
    (void)s1; (void)s2;
    my_free(a);
    my_free(b);                        /* holes at 0:32 and 0:288; position 0:544 */
    my_set_policy(NEXT_FIT);
    my_malloc(10);                     /* first block ending after 0:544: the free tail */
    my_malloc(10);                     /* continues after it */
    my_heap_dump();
}

/* Next fit spans regions: a block in a later region is "after" the position. */
static void next_fit_across_regions(void) {
    my_set_policy(NEXT_FIT);
    void *a = my_malloc(60000);        /* region 0 */
    my_malloc(60000);                  /* region 1; position 1:60080 */
    my_free(a);                        /* region 0 is free again */
    my_malloc(100);                    /* region 1 tail ends after the position: placed there */
    my_heap_dump();
}

/* Best fit: smallest fitting block, earliest among equals. */
static void best_fit_ties(void) {
    void *h1 = my_malloc(200), *s1 = my_malloc(10);   /* 240 */
    void *h2 = my_malloc(100), *s2 = my_malloc(10);   /* 144 */
    void *h3 = my_malloc(200), *s3 = my_malloc(10);   /* 240 */
    void *h4 = my_malloc(100), *s4 = my_malloc(10);   /* 144 */
    (void)s1; (void)s2; (void)s3; (void)s4;
    my_free(h1); my_free(h2); my_free(h3); my_free(h4);
    my_set_policy(BEST_FIT);
    my_malloc(100);                    /* 144: h2 (earliest of the two 144s) */
    my_malloc(100);                    /* 144: h4 */
    my_malloc(150);                    /* 192: h1 (earliest 240), leftover 48 not split */
    my_heap_dump();
}

/* Worst fit: largest block, earliest among equals. */
static void worst_fit_ties(void) {
    void *a = my_malloc(30000), *s1 = my_malloc(10);
    void *b = my_malloc(30000), *s2 = my_malloc(10);
    (void)s1; (void)s2;
    my_free(a);
    my_free(b);                        /* two free blocks of 30048, tail is smaller */
    my_set_policy(WORST_FIT);
    my_malloc(10);                     /* earliest largest: 0:32 */
    my_malloc(10);                     /* now 0:30208's 30048 is the largest */
    my_heap_dump();
}

/* Worst fit grows the heap when even the largest block is too small. */
static void worst_fit_grows(void) {
    my_set_policy(WORST_FIT);
    my_malloc(10);
    my_malloc(70000);                  /* nothing fits: region 1 of 131072 */
    my_malloc(40000);                  /* largest is region 0's tail (65376 > 60992) */
    my_heap_dump();
}

/* The policy can change between any two calls, and applies to calloc/realloc too. */
static void policy_switching(void) {
    void *h1 = my_malloc(300), *s1 = my_malloc(10);   /* 0:32 size 352 */
    void *h2 = my_malloc(100), *s2 = my_malloc(10);   /* 144 */
    (void)s1; (void)s2;
    my_free(h1); my_free(h2);
    my_set_policy(BEST_FIT);
    my_calloc(10, 10);                 /* 144 hole */
    my_set_policy(FIRST_FIT);
    my_calloc(10, 10);                 /* first hole: 0:32 */
    my_heap_dump();
}

/* ---------------- Part 3: calloc and realloc ---------------- */

static void calloc_limits(void) {
    void *a = my_malloc(1);
    R(my_calloc(0, 5));
    R(my_calloc(5, 0));
    R(my_calloc(SIZE_MAX, 2));                 /* overflows size_t */
    R(my_calloc(SIZE_MAX / 2 + 1, 4));         /* overflows to a small number */
    R(my_calloc((1 << 15) + 1, 1 << 15));      /* just over 2^30 */
    R(my_calloc(3, 7));
    my_free(a);
    my_heap_dump();
}

/* calloc must zero memory even when it reuses a dirty block. */
static void calloc_zeroes_reused(void) {
    unsigned char *a = my_malloc(500);
    memset(a, 0xFF, 500);
    void *s = my_malloc(10);
    (void)s;
    my_free(a);
    unsigned char *b = my_calloc(50, 10);
    int bad = 0;
    for (int i = 0; i < 500; i++) bad |= b[i];
    printf("calloc reused block %s, zeroed: %s\n", b == a ? "same" : "different", bad ? "NO" : "yes");
    my_heap_dump();
}

/* realloc(NULL, n) is malloc; realloc(p, 0) frees and returns NULL. */
static void realloc_null_and_zero(void) {
    void *a = my_realloc(NULL, 50);
    R(a);
    my_heap_dump();
    R(my_realloc(a, 0));
    my_heap_dump();
}

/* Shrink in place keeps the pointer; a tail of >= 128 is split and merged forward. */
static void realloc_shrink(void) {
    char *a = my_malloc(1000);         /* 1040 */
    fill(a, 1000, 1);
    void *s = my_malloc(10);
    (void)s;
    char *b = my_realloc(a, 500);      /* B = 544, tail 496 split */
    printf("same pointer: %s\n", a == b ? "yes" : "no");
    check("shrink", b, 500, 1);
    char *c = my_realloc(b, 450);      /* B = 496, tail 48: no split */
    printf("same pointer: %s\n", b == c ? "yes" : "no");
    my_heap_dump();
}

/* The tail cut off by a shrink merges with a free block after it. */
static void realloc_shrink_merges_tail(void) {
    char *a = my_malloc(1000);         /* 0:32 size 1040 */
    my_realloc(a, 100);                /* B = 144; tail 896 merges with the free rest */
    my_heap_dump();
}

/* A larger n that still fits in the same block size stays in place. */
static void realloc_same_block(void) {
    char *a = my_malloc(100);          /* 144 */
    fill(a, 100, 3);
    char *b = my_realloc(a, 104);      /* still 144 */
    printf("same pointer: %s\n", a == b ? "yes" : "no");
    check("same block", b, 100, 3);
    my_heap_dump();
}

/* Grow in place into the next free block, splitting off the leftover. */
static void realloc_grow_split(void) {
    char *a = my_malloc(100);          /* 0:32 144 */
    fill(a, 100, 5);
    char *b = my_realloc(a, 300);      /* B = 352, next is the free tail */
    printf("same pointer: %s\n", a == b ? "yes" : "no");
    check("grow", b, 100, 5);
    my_heap_dump();
}

/* Moving: the data is copied, the old block is freed (and merges). */
static void realloc_move(void) {
    char *a = my_malloc(100);          /* 0:32 */
    fill(a, 100, 9);
    void *s = my_malloc(10);           /* blocks growth in place */
    (void)s;
    char *b = my_realloc(a, 1000);
    printf("moved: %s\n", a != b ? "yes" : "no");
    check("move", b, 100, 9);
    my_heap_dump();
}

/* The block before ptr is never used, even when it is free and large. */
static void realloc_never_uses_previous(void) {
    void *a = my_malloc(1000);         /* 0:32 1040 */
    char *b = my_malloc(100);          /* 0:1072 144 */
    void *c = my_malloc(10);           /* 0:1216 */
    (void)c;
    fill(b, 100, 11);
    my_free(a);
    char *d = my_realloc(b, 200);      /* must not merge backwards; first fit puts it at 0:32 */
    check("move", d, 100, 11);
    my_heap_dump();
}

/* The old block is never reused for the new one: the new block is placed
 * BEFORE the old one is freed. Freeing first would merge the old block with
 * the free block before it (144 + 144 = 288 >= 240) and first fit would put
 * the new block at 0:32. Correct: it goes to the tail, then the old merges. */
static void realloc_old_block_not_reused(void) {
    void *x = my_malloc(100);          /* 0:32 144 */
    char *a = my_malloc(100);          /* 0:176 144 */
    void *t = my_malloc(10);           /* 0:320 */
    (void)t;
    my_free(x);                        /* 0:32 free 144 (too small for 240 alone) */
    char *b = my_realloc(a, 200);      /* B = 240, next is allocated: moves */
    printf("moved: %s\n", a != b ? "yes" : "no");
    my_heap_dump();
}

/* realloc with n > 2^30 returns NULL and leaves the block alone. */
static void realloc_too_big(void) {
    char *a = my_malloc(100);
    fill(a, 100, 13);
    R(my_realloc(a, (1UL << 30) + 1));
    check("unchanged", a, 100, 13);
    my_heap_dump();
}

/* realloc with bad pointers reports the error and returns NULL. */
static void realloc_bad_pointers(void) {
    char *a = my_malloc(10), *b = my_malloc(10);
    (void)b;
    my_free(a);
    R(my_realloc(a, 50));              /* double free */
    R(my_realloc(b + 8, 50));          /* invalid */
    int local;
    R(my_realloc(&local, 50));         /* invalid */
    my_heap_dump();
}

/* Moving realloc updates the next-fit position (it is a new allocation). */
static void realloc_move_sets_next_fit(void) {
    void *a = my_malloc(10);           /* 0:32 */
    void *s = my_malloc(10);           /* 0:160 */
    void *t = my_malloc(10);           /* 0:288 */
    void *h = my_malloc(10);           /* 0:416, becomes a hole */
    void *u = my_malloc(10);           /* 0:544 */
    (void)s; (void)u;
    my_free(h);                        /* hole 0:416 */
    my_free(t);
    my_malloc(10);                     /* first fit puts it back at 0:288: position 0:416 */
    my_set_policy(NEXT_FIT);
    my_realloc(a, 1000);               /* hole too small: moves to the tail 0:672;
                                          position becomes 0:1712 */
    my_malloc(10);                     /* placed at 0:1712. If the move did not set the
                                          position (still 0:416), it would go to 0:416 */
    my_heap_dump();
}

/* ---------------- Part 4: errors ---------------- */

/* All kinds of invalid pointer, and double frees of free / merged blocks. */
static void invalid_pointers(void) {
    char *a = my_malloc(10), *b = my_malloc(10), *c = my_malloc(100);
    (void)c;
    int local;
    my_free(&local);                   /* stack */
    my_free(&global_var);              /* global */
    my_free(a + 1);                    /* inside a block */
    my_free(a + 16);                   /* inside, 16-aligned */
    my_free(a - 32);                   /* the region header area */
    my_free(a - 16);
    my_free(c + 96);                   /* inside c, at its guard bytes */
    my_free(a + 70000);                /* beyond region 0 */
    my_free(a);
    my_free(a);                        /* double free of a free block */
    my_free(b);
    my_free(a);                        /* a + b merged: still a double free (block 0:32) */
    my_free(b);                        /* b is no longer a block start: invalid */
    my_heap_dump();
}

/* Guard bytes are the 8 bytes right after n, not the last 8 of the block. */
static void guard_position(void) {
    char *a = my_malloc(10);           /* block 128: user 10 + guard 8, then padding */
    a[18] = 'p';                       /* padding: not a guard byte, no error */
    my_free(a);
    char *b = my_malloc(10);
    b[10] = 'g';                       /* first guard byte */
    my_free(b);
    char *c = my_malloc(10);
    c[17] = 'g';                       /* last guard byte */
    my_free(c);
}

/* Several changed guard bytes give one message per call. */
static void guard_one_message(void) {
    char *a = my_malloc(10);
    memset(a + 10, 0, 8);
    my_free(a);
    my_heap_dump();
}

/* Overflow is reported by realloc, the call carries on, and the guards then
 * follow the new n (so the next free is clean). Also for shrink and move. */
static void guard_realloc(void) {
    char *a = my_malloc(100);
    a[100] = 0;
    a = my_realloc(a, 200);            /* grow in place: overflow, then OK */
    a[200] = 0;
    a = my_realloc(a, 20);             /* shrink: overflow, then OK */
    void *s = my_malloc(10);
    (void)s;
    a[20] = 0;
    a = my_realloc(a, 5000);           /* move: overflow reported once only */
    my_free(a);                        /* clean */
    my_heap_dump();
}

/* After realloc the guard follows the new n: writing at the old n is fine. */
static void guard_follows_new_n(void) {
    char *a = my_malloc(10);
    a = my_realloc(a, 20);             /* same block, n = 20 */
    a[10] = 'x';                       /* old guard position, now user data */
    my_free(a);                        /* no error */
    my_heap_dump();
}

/* Freed bytes become 0xDE while delayed; delayed blocks never merge. */
static void delay_basics(void) {
    my_set_delay(3);
    unsigned char *a = my_malloc(10), *b = my_malloc(10), *c = my_malloc(10);
    (void)c;
    my_free(a);
    my_free(b);
    printf("a[0] = 0x%02X, b[9] = 0x%02X\n", a[0], b[9]);
    my_heap_dump();
    my_free(a);                        /* double free of a delayed block */
    my_free(b + 1);                    /* invalid */
}

/* Delayed blocks are never reused: requests go elsewhere. */
static void delay_never_reused(void) {
    my_set_delay(1);
    void *a = my_malloc(10), *s = my_malloc(10);
    (void)s;
    my_free(a);                        /* delayed at 0:32 */
    my_malloc(10);                     /* first fit must skip it */
    my_heap_dump();
}

/* The queue keeps the k most recent frees and evicts in FIFO order. */
static void delay_fifo(void) {
    my_set_delay(2);
    void *p[5];
    for (int i = 0; i < 5; i++) p[i] = my_malloc(10);
    for (int i = 0; i < 5; i++) my_free(p[i]);   /* p0..p2 evicted, p3 p4 delayed */
    my_heap_dump();
}

/* Lowering k evicts from the front (checking 0xDE); raising k keeps blocks. */
static void delay_lowering(void) {
    my_set_delay(4);
    char *p[4];
    for (int i = 0; i < 4; i++) p[i] = my_malloc(10);
    void *s = my_malloc(10);
    (void)s;
    for (int i = 0; i < 4; i++) my_free(p[i]);
    p[0][0] = 1;                       /* write after free on the oldest */
    p[2][9] = 1;                       /* and on the third */
    my_set_delay(8);                   /* nothing happens */
    my_heap_dump();
    my_set_delay(1);                   /* evicts p0 (error), p1, p2 (error) */
    my_heap_dump();
    my_set_delay(0);                   /* evicts p3 */
    my_heap_dump();
}

/* Overflow check happens before the block is delayed; with k > 0 realloc
 * moves and realloc(p, 0) also go through the queue. */
static void delay_with_realloc(void) {
    my_set_delay(2);
    char *a = my_malloc(10);
    void *s = my_malloc(10);
    (void)s;
    a[10] = 0;
    char *b = my_realloc(a, 500);      /* overflow, move, old block delayed */
    char *c = my_malloc(20);
    my_realloc(c, 0);                  /* delayed too */
    (void)b;
    my_heap_dump();
}

/* A delayed next block is not free: realloc cannot grow into it. */
static void delay_blocks_grow(void) {
    my_set_delay(1);
    char *a = my_malloc(10), *b = my_malloc(10);
    my_free(b);                        /* b delayed right after a */
    char *c = my_realloc(a, 100);      /* cannot grow into b: moves */
    printf("moved: %s\n", a != c ? "yes" : "no");
    my_heap_dump();
}

/* Evicted blocks merge with free neighbours as normal. */
static void delay_eviction_merges(void) {
    my_set_delay(1);
    void *a = my_malloc(10), *b = my_malloc(10), *c = my_malloc(10);
    (void)c;
    my_free(a);                        /* delayed */
    my_free(b);                        /* a evicted (free), b delayed */
    my_set_delay(0);                   /* b evicted: merges with a */
    my_heap_dump();
}

static void leak_none(void) {
    void *a = my_malloc(10);
    my_free(a);
    my_leak_report();
}

/* Delayed and free blocks are not leaks; order is heap order across regions. */
static void leak_order(void) {
    my_set_delay(1);
    void *a = my_malloc(70000);        /* region 1 */
    void *b = my_malloc(5);            /* 0:32 */
    void *c = my_malloc(6);            /* 0:160 */
    void *d = my_malloc(7);            /* 0:288 */
    (void)a; (void)d;
    my_free(b);                        /* delayed */
    my_realloc(c, 9);                  /* still allocated, n = 9 */
    my_leak_report();
}

/* ---------------- Part 5: buddy ---------------- */

static void buddy_empty(void) {
    my_set_policy(BUDDY);
    my_heap_dump();                    /* nothing yet */
    R(my_malloc(0));
    R(my_malloc(1048576 - 15));        /* n + 16 > 2^20 */
}

static void buddy_sizes(void) {
    my_set_policy(BUDDY);
    my_malloc(1);      /* 32 */
    my_malloc(16);     /* 32 */
    my_malloc(17);     /* 64 */
    my_malloc(48);     /* 64 */
    my_malloc(49);     /* 128 */
    my_malloc(1000);   /* 1024 */
    my_heap_dump();
}

static void buddy_full(void) {
    my_set_policy(BUDDY);
    void *a = my_malloc(1048576 - 16); /* the whole region */
    R(a);
    R(my_malloc(1));                   /* full */
    my_heap_dump();
    my_free(a);
    my_heap_dump();
}

/* Freeing merges buddies repeatedly, back to one block. */
static void buddy_merge_cascade(void) {
    my_set_policy(BUDDY);
    void *p[4];
    for (int i = 0; i < 4; i++) p[i] = my_malloc(16);   /* 0, 32, 64, 96 */
    my_free(p[0]);
    my_free(p[2]);
    my_heap_dump();                    /* 0 and 64 free, but not merged */
    my_free(p[1]);                     /* 0+32 -> 64 at 0 */
    my_heap_dump();
    my_free(p[3]);                     /* all the way back up */
    my_heap_dump();
}

/* Two free neighbours of the same size that are NOT buddies do not merge. */
static void buddy_not_buddies(void) {
    my_set_policy(BUDDY);
    void *p[4];
    for (int i = 0; i < 4; i++) p[i] = my_malloc(16);   /* 0, 32, 64, 96 */
    my_free(p[1]);                     /* 32 */
    my_free(p[2]);                     /* 64: neighbour of 32, but its buddy is 96 */
    my_heap_dump();
}

/* Running out: later requests fail but frees still work. */
static void buddy_exhaust(void) {
    my_set_policy(BUDDY);
    void *p[40];
    int got = 0;
    for (int i = 0; i < 40; i++) {
        p[i] = my_malloc(65536 - 16);  /* 64 KiB blocks: 16 fit */
        if (p[i]) got++;
    }
    printf("got %d blocks\n", got);
    for (int i = 0; i < 16; i += 2) my_free(p[i]);
    R(my_malloc(100000));              /* needs 128 KiB: no two free 64 KiB are buddies */
    R(my_malloc(60000));               /* 64 KiB: fits in a hole */
    my_heap_dump();
}

typedef struct { const char *name; void (*fn)(void); } Case;
static const Case cases[] = {
    {"empty_heap", empty_heap}, {"malloc_limits", malloc_limits}, {"block_sizes", block_sizes},
    {"split_threshold", split_threshold}, {"coalesce_both_sides", coalesce_both_sides},
    {"regions_never_merge", regions_never_merge}, {"growth_sizes", growth_sizes},
    {"exact_fit", exact_fit}, {"next_fit_wraps", next_fit_wraps},
    {"next_fit_continues", next_fit_continues}, {"next_fit_across_regions", next_fit_across_regions},
    {"best_fit_ties", best_fit_ties}, {"worst_fit_ties", worst_fit_ties},
    {"worst_fit_grows", worst_fit_grows}, {"policy_switching", policy_switching},
    {"calloc_limits", calloc_limits}, {"calloc_zeroes_reused", calloc_zeroes_reused},
    {"realloc_null_and_zero", realloc_null_and_zero}, {"realloc_shrink", realloc_shrink},
    {"realloc_shrink_merges_tail", realloc_shrink_merges_tail}, {"realloc_same_block", realloc_same_block},
    {"realloc_grow_split", realloc_grow_split}, {"realloc_move", realloc_move},
    {"realloc_never_uses_previous", realloc_never_uses_previous},
    {"realloc_old_block_not_reused", realloc_old_block_not_reused},
    {"realloc_too_big", realloc_too_big}, {"realloc_bad_pointers", realloc_bad_pointers},
    {"realloc_move_sets_next_fit", realloc_move_sets_next_fit},
    {"invalid_pointers", invalid_pointers}, {"guard_position", guard_position},
    {"guard_one_message", guard_one_message}, {"guard_realloc", guard_realloc},
    {"guard_follows_new_n", guard_follows_new_n}, {"delay_basics", delay_basics},
    {"delay_never_reused", delay_never_reused}, {"delay_fifo", delay_fifo},
    {"delay_lowering", delay_lowering}, {"delay_with_realloc", delay_with_realloc},
    {"delay_blocks_grow", delay_blocks_grow}, {"delay_eviction_merges", delay_eviction_merges},
    {"leak_none", leak_none}, {"leak_order", leak_order},
    {"buddy_empty", buddy_empty}, {"buddy_sizes", buddy_sizes}, {"buddy_full", buddy_full},
    {"buddy_merge_cascade", buddy_merge_cascade}, {"buddy_not_buddies", buddy_not_buddies},
    {"buddy_exhaust", buddy_exhaust},
};

int main(int argc, char **argv) {
    if (argc < 2) return 2;
    if (!strcmp(argv[1], "--list")) {
        for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) printf("%s\n", cases[i].name);
        return 0;
    }
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++)
        if (!strcmp(argv[1], cases[i].name)) { cases[i].fn(); return 0; }
    return 2;
}
