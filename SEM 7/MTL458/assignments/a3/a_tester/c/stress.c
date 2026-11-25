/* Stress / performance workloads (Part 6).
 *
 * usage: stress <workload> <policy> [holes]
 *
 * Every workload first builds a heap with many free blocks ("holes", at
 * least 100,000 by default), then makes at least 100,000 more calls, as the
 * PDF describes. The program keeps its own record of what is allocated, and
 * at the end prints to stdout:
 *     the heap dump, then the leak report
 * and to stderr:
 *     EXPECT <live blocks> <live requested bytes>
 *     PHASE2 <seconds spent in the timed second phase>
 *     CALLS <number of calls in the second phase>
 * so the tester can check the leak report against the program's own count,
 * check the dump for consistency, and measure how the time grows with N. */
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "myalloc.h"

typedef struct { unsigned char *p; size_t n; } Slot;

static Slot *slots;
static long nslots;
static unsigned long long rng = 88172645463325252ULL;
static long calls;
static long phase2_calls = -1;   /* calls made in the timed phase */

static unsigned long next_rand(void) {
    rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17;
    return (unsigned long)(rng >> 11);
}

static double now(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + t.tv_nsec / 1e9;
}

/* Write a recognisable byte pattern; verify it later to catch corruption. */
static void stamp(long i) {
    Slot *s = &slots[i];
    size_t k = s->n < 16 ? s->n : 16;
    for (size_t j = 0; j < k; j++) s->p[j] = (unsigned char)(i * 31 + j);
    if (s->n > 16) s->p[s->n - 1] = (unsigned char)(i ^ 0x5A);
}

static int verify(long i) {
    Slot *s = &slots[i];
    size_t k = s->n < 16 ? s->n : 16;
    for (size_t j = 0; j < k; j++)
        if (s->p[j] != (unsigned char)(i * 31 + j)) return 0;
    if (s->n > 16 && s->p[s->n - 1] != (unsigned char)(i ^ 0x5A)) return 0;
    return 1;
}

static int corrupt = 0;

static void do_malloc(long i, size_t n) {
    slots[i].p = my_malloc(n);
    slots[i].n = n;
    calls++;
    if (!slots[i].p) { fprintf(stderr, "UNEXPECTED NULL from my_malloc(%zu)\n", n); exit(3); }
    stamp(i);
}

static void do_free(long i) {
    if (!verify(i)) corrupt++;
    my_free(slots[i].p);
    slots[i].p = NULL;
    calls++;
}

static void do_realloc(long i, size_t n) {
    if (!verify(i)) corrupt++;
    size_t keep = slots[i].n < n ? slots[i].n : n;
    unsigned char first = slots[i].p[0];
    unsigned char *q = my_realloc(slots[i].p, n);
    calls++;
    if (!q) { fprintf(stderr, "UNEXPECTED NULL from my_realloc(%zu)\n", n); exit(3); }
    if (keep && q[0] != first) corrupt++;
    slots[i].p = q;
    slots[i].n = n;
    stamp(i);
}

/* Phase 1 for the fit policies: allocate 2*holes blocks and free every other
 * one, leaving `holes` free blocks that cannot merge. */
static void build_holes(long holes, size_t max_small) {
    nslots = 2 * holes + 1000;
    slots = calloc((size_t)nslots, sizeof(Slot));   /* the tester's own memory: libc */
    for (long i = 0; i < 2 * holes; i++) do_malloc(i, 1 + next_rand() % max_small);
    for (long i = 0; i < 2 * holes; i += 2) do_free(i);
}

static void report(double phase2) {
    long live = 0;
    unsigned long long bytes = 0;
    for (long i = 0; i < nslots; i++)
        if (slots[i].p) { live++; bytes += slots[i].n; }
    fflush(stdout);
    my_heap_dump();
    my_leak_report();
    fflush(stdout);
    fprintf(stderr, "EXPECT %ld %llu\n", live, bytes);
    fprintf(stderr, "PHASE2 %.4f\n", phase2);
    fprintf(stderr, "CALLS %ld\n", calls);
    fprintf(stderr, "PHASE2_CALLS %ld\n", phase2_calls);
    fprintf(stderr, "CORRUPT %d\n", corrupt);
}

/* mixed: random malloc / free / realloc / calloc on top of the holes. */
static void mixed(long holes, long ops) {
    build_holes(holes, 400);
    double t0 = now();
    long start_calls = calls;
    for (long k = 0; k < ops; k++) {
        long i = (long)(next_rand() % (unsigned long)(2 * holes));
        unsigned long r = next_rand() % 10;
        if (!slots[i].p) {
            if (r < 2) {
                size_t n = 1 + next_rand() % 600;
                slots[i].p = my_calloc(n, 1);
                slots[i].n = n;
                calls++;
                if (!slots[i].p) { fprintf(stderr, "UNEXPECTED NULL from my_calloc\n"); exit(3); }
                for (size_t j = 0; j < n; j++) if (slots[i].p[j]) { corrupt++; break; }
                stamp(i);
            } else {
                do_malloc(i, 1 + next_rand() % 2000);
            }
        } else if (r < 6) {
            do_free(i);
        } else {
            do_realloc(i, 1 + next_rand() % 3000);
        }
    }
    phase2_calls = calls - start_calls;
    report(now() - t0);
}

/* big_requests: every request is larger than every hole, so a search that
 * walks the free blocks one by one visits all of them each time. */
static void big_requests(long holes, long ops) {
    build_holes(holes, 80);                        /* holes are 128 bytes */
    long base = 2 * holes;
    double t0 = now();
    long start_calls = calls;
    for (long k = 0; k < ops / 2; k++) {
        do_malloc(base, 500 + next_rand() % 500);
        do_free(base);
    }
    phase2_calls = calls - start_calls;
    report(now() - t0);
}

/* free_heavy: frees blocks in random order. Phase 2 repeatedly frees a random
 * live block (it merges with the holes on both sides) and allocates it again,
 * so the number of free blocks stays about the same while every call does a
 * coalescing free. Exercises my_free, which must be O(log N) under every policy. */
static void free_heavy(long holes, long ops) {
    build_holes(holes, 200);
    double t0 = now();
    long start_calls = calls;
    for (long k = 0; k < ops / 2; k++) {
        long i = 1 + 2 * (long)(next_rand() % (unsigned long)holes);   /* a live (odd) slot */
        if (slots[i].p) do_free(i);
        do_malloc(i, 1 + next_rand() % 200);
    }
    phase2_calls = calls - start_calls;
    report(now() - t0);
}

/* delayed: the mixed workload with the largest delay queue. */
static void delayed(long holes, long ops) {
    my_set_delay(64);
    mixed(holes, ops);
}

/* buddy: random small allocations and frees inside the 1 MiB buddy region. */
static void buddy(long holes, long ops) {
    (void)holes;
    nslots = 20000;
    slots = calloc((size_t)nslots, sizeof(Slot));
    for (long i = 0; i < nslots; i += 1) {         /* fill most of the region */
        size_t n = 1 + next_rand() % 40;
        slots[i].p = my_malloc(n);
        slots[i].n = n;
        calls++;
        if (slots[i].p) stamp(i); else slots[i].n = 0;
    }
    double t0 = now();
    for (long k = 0; k < ops; k++) {
        long i = (long)(next_rand() % (unsigned long)nslots);
        if (slots[i].p) { do_free(i); }
        else {
            size_t n = 1 + next_rand() % ((next_rand() % 8) ? 100 : 3000);
            slots[i].p = my_malloc(n);
            slots[i].n = n;
            calls++;
            if (slots[i].p) stamp(i); else slots[i].n = 0;
        }
    }
    double t = now() - t0;
    long live = 0;
    for (long i = 0; i < nslots; i++) if (slots[i].p) live++;
    fflush(stdout);
    my_heap_dump();
    fflush(stdout);
    fprintf(stderr, "EXPECT %ld 0\nPHASE2 %.4f\nCALLS %ld\nCORRUPT %d\n", live, t, calls, corrupt);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: stress <mixed|big_requests|free_heavy|delayed|buddy> <policy> [holes] [ops]\n");
        return 2;
    }
    const char *w = argv[1];
    int policy = atoi(argv[2]);
    long holes = argc > 3 ? atol(argv[3]) : 110000;
    long ops = argc > 4 ? atol(argv[4]) : 200000;
    my_set_policy(policy);
    if (!strcmp(w, "mixed")) mixed(holes, ops);
    else if (!strcmp(w, "big_requests")) big_requests(holes, ops);
    else if (!strcmp(w, "free_heavy")) free_heavy(holes, ops);
    else if (!strcmp(w, "delayed")) delayed(holes, ops);
    else if (!strcmp(w, "buddy")) buddy(holes, ops);
    else return 2;
    return 0;
}
