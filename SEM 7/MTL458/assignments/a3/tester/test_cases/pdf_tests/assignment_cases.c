/*
 * Small drivers for the literal examples in the Assignment 3 PDF.
 * Compile with a3.c and myalloc.h; each case runs in a fresh process.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "myalloc.h"

static void part1(void) {
    void *a = my_malloc(10);
    void *b = my_malloc(100);
    void *c = my_malloc(10);
    my_heap_dump();
    my_free(b);
    my_free(a);
    void *d = my_malloc(100000);
    (void)c;
    (void)d;
    my_heap_dump();
}

static void setup_policy_example(void) {
    void *a = my_malloc(248);
    void *x1 = my_malloc(10);
    void *b = my_malloc(120);
    void *x2 = my_malloc(10);
    void *c = my_malloc(360);
    void *x3 = my_malloc(10);
    my_free(b);
    void *y = my_malloc(120);
    my_free(y);
    my_free(a);
    my_free(c);
    (void)x1;
    (void)x2;
    (void)x3;
}

static void policy_case(int policy) {
    setup_policy_example();
    my_set_policy(policy);
    void *p = my_malloc(100);
    (void)p;
    my_heap_dump();
}

static void realloc_case(void) {
    void *p = my_malloc(100);
    void *q = my_malloc(200);
    void *r = my_malloc(10);
    my_free(q);
    p = my_realloc(p, 250);
    my_heap_dump();
    p = my_realloc(p, 20);
    my_heap_dump();
    (void)p;
    (void)r;
}

static void delay_case(void) {
    my_set_delay(2);
    unsigned char *a = my_malloc(10);
    unsigned char *b = my_malloc(10);
    unsigned char *c = my_malloc(10);
    unsigned char *d = my_malloc(10);
    my_free(a);
    my_free(b);
    b[3] = 'X';
    my_free(c);
    my_free(d);
    my_heap_dump();
    my_free(a);
    my_free(b);
}

static void leak_case(void) {
    void *a = my_malloc(100);
    void *b = my_malloc(10);
    void *c = my_malloc(10);
    my_free(b);
    my_leak_report();
    (void)a;
    (void)c;
}

static void buddy_case(void) {
    my_set_policy(BUDDY);
    void *a = my_malloc(40);
    void *b = my_malloc(100);
    void *c = my_malloc(40);
    my_heap_dump();
    (void)a;
    (void)b;
    (void)c;
}

static void allocation_bounds_case(void) {
    if (my_malloc(0) || my_malloc(((size_t)1 << 30) + 1) ||
        my_calloc(0, 4) || my_calloc(4, 0) ||
        my_calloc(SIZE_MAX, 2) || my_calloc(((size_t)1 << 30), 2)) {
        puts("unexpected non-NULL allocation");
        return;
    }
    my_heap_dump();
}

static void calloc_case(void) {
    unsigned char *p = my_calloc(3, 4);
    if (!p) {
        puts("calloc returned NULL");
        return;
    }
    for (size_t i = 0; i < 12; ++i) {
        if (p[i] != 0) {
            puts("calloc did not zero memory");
            return;
        }
    }
    my_heap_dump();
}

static void pointer_errors_case(void) {
    int stack_value = 7;
    unsigned char *p = my_malloc(20);
    my_free(&stack_value);
    my_free(p + 1);
    my_free(p);
    my_free(p);
    my_realloc(p, 10);
    my_realloc(&stack_value, 12);
    my_heap_dump();
}

static void overflow_case(void) {
    unsigned char *p = my_malloc(4);
    p[4] = 0;
    my_free(p);
    my_heap_dump();
}

static void realloc_in_place_case(void) {
    unsigned char *p = my_malloc(100);
    void *q = my_malloc(100);
    for (size_t i = 0; i < 100; ++i) p[i] = (unsigned char)i;
    my_free(q);
    p = my_realloc(p, 500);
    for (size_t i = 0; i < 100; ++i) {
        if (p[i] != (unsigned char)i) {
            puts("realloc lost data while growing");
            return;
        }
    }
    my_heap_dump();
    p = my_realloc(p, 20);
    for (size_t i = 0; i < 20; ++i) {
        if (p[i] != (unsigned char)i) {
            puts("realloc lost data while shrinking");
            return;
        }
    }
    my_heap_dump();
}

static void realloc_move_case(void) {
    unsigned char *p = my_malloc(20);
    void *q = my_malloc(20);
    void *r = my_malloc(20);
    for (size_t i = 0; i < 20; ++i) p[i] = (unsigned char)(i + 1);
    p = my_realloc(p, 1000);
    if (!p) {
        puts("realloc move returned NULL");
        return;
    }
    for (size_t i = 0; i < 20; ++i) {
        if (p[i] != (unsigned char)(i + 1)) {
            puts("realloc move lost data");
            return;
        }
    }
    my_heap_dump();
    (void)q;
    (void)r;
}

static void realloc_zero_and_oversize_case(void) {
    void *p = my_malloc(10);
    void *q = my_malloc(10);
    void *too_large = my_realloc(q, ((size_t)1 << 30) + 1);
    if (too_large) puts("oversized realloc unexpectedly succeeded");
    my_heap_dump();
    if (my_realloc(p, 0) != NULL) puts("zero realloc did not return NULL");
    my_heap_dump();
}

static void delay_lower_case(void) {
    my_set_delay(3);
    void *a = my_malloc(10);
    void *b = my_malloc(10);
    void *c = my_malloc(10);
    my_free(a);
    my_free(b);
    my_free(c);
    my_set_delay(1);
    my_heap_dump();
}

static void delay_capacity_case(void) {
    void *blocks[65];
    my_set_delay(64);
    for (size_t i = 0; i < 65; ++i) blocks[i] = my_malloc(1);
    for (size_t i = 0; i < 65; ++i) my_free(blocks[i]);
    my_heap_dump();
}

static void next_fit_coalesced_cursor_case(void) {
    my_set_policy(NEXT_FIT);
    void *a = my_malloc(10);
    void *b = my_malloc(10);
    void *c = my_malloc(10);
    void *d = my_malloc(10);
    void *e = my_malloc(10);
    my_free(a);
    my_free(e);
    void *p = my_malloc(10);
    (void)b;
    (void)c;
    (void)d;
    (void)p;
    my_heap_dump();
}

static void buddy_merge_case(void) {
    my_set_policy(BUDDY);
    void *a = my_malloc(1);
    void *b = my_malloc(1);
    void *c = my_malloc(1);
    my_free(a);
    my_free(b);
    my_free(c);
    my_heap_dump();
}

static void first_large_region_case(void) {
    void *p = my_malloc(100000);
    if (!p) puts("large first allocation failed");
    my_heap_dump();
}

static void realloc_guard_oversize_case(void) {
    unsigned char *p = my_malloc(4);
    if (!p) return;
    p[4] = 0;
    if (my_realloc(p, ((size_t)1 << 30) + 1) != NULL)
        puts("oversized realloc unexpectedly succeeded");
    my_heap_dump();
}

static void realloc_delay_case(void) {
    my_set_delay(1);
    void *p = my_malloc(10);
    my_realloc(p, 0);
    my_heap_dump();
}

static void delay_lower_write_case(void) {
    my_set_delay(2);
    unsigned char *p = my_malloc(10);
    (void)my_malloc(10);
    my_free(p);
    p[0] = 1;
    my_set_delay(0);
    my_heap_dump();
}

static void buddy_limit_case(void) {
    my_set_policy(BUDDY);
    if (my_malloc(1048561) != NULL) puts("oversized buddy request succeeded");
    void *p = my_malloc(1048560);
    if (!p) puts("maximum buddy request failed");
    if (my_malloc(1) != NULL) puts("exhausted buddy arena allocated");
    my_heap_dump();
}

static void policy_tie_case(int policy) {
    void *a = my_malloc(10);
    void *barrier1 = my_malloc(10);
    void *b = my_malloc(10);
    void *barrier2 = my_malloc(10);
    void *c = my_malloc(10);
    void *tail = my_malloc(64700);
    my_free(a);
    my_free(b);
    my_free(c);
    my_set_policy(policy);
    (void)my_malloc(10);
    my_heap_dump();
    (void)barrier1;
    (void)barrier2;
    (void)tail;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: assignment_cases CASE\n");
        return 2;
    }
    if (strcmp(argv[1], "part1") == 0) part1();
    else if (strcmp(argv[1], "policy_first") == 0) policy_case(FIRST_FIT);
    else if (strcmp(argv[1], "policy_next") == 0) policy_case(NEXT_FIT);
    else if (strcmp(argv[1], "policy_best") == 0) policy_case(BEST_FIT);
    else if (strcmp(argv[1], "policy_worst") == 0) policy_case(WORST_FIT);
    else if (strcmp(argv[1], "realloc") == 0) realloc_case();
    else if (strcmp(argv[1], "delay") == 0) delay_case();
    else if (strcmp(argv[1], "leaks") == 0) leak_case();
    else if (strcmp(argv[1], "buddy") == 0) buddy_case();
    else if (strcmp(argv[1], "allocation_bounds") == 0) allocation_bounds_case();
    else if (strcmp(argv[1], "calloc") == 0) calloc_case();
    else if (strcmp(argv[1], "pointer_errors") == 0) pointer_errors_case();
    else if (strcmp(argv[1], "overflow") == 0) overflow_case();
    else if (strcmp(argv[1], "realloc_in_place") == 0) realloc_in_place_case();
    else if (strcmp(argv[1], "realloc_move") == 0) realloc_move_case();
    else if (strcmp(argv[1], "realloc_zero_oversize") == 0) realloc_zero_and_oversize_case();
    else if (strcmp(argv[1], "delay_lower") == 0) delay_lower_case();
    else if (strcmp(argv[1], "delay_capacity") == 0) delay_capacity_case();
    else if (strcmp(argv[1], "nextfit_coalesced_cursor") == 0) next_fit_coalesced_cursor_case();
    else if (strcmp(argv[1], "buddy_merge") == 0) buddy_merge_case();
    else if (strcmp(argv[1], "first_large_region") == 0) first_large_region_case();
    else if (strcmp(argv[1], "realloc_guard_oversize") == 0) realloc_guard_oversize_case();
    else if (strcmp(argv[1], "realloc_delay") == 0) realloc_delay_case();
    else if (strcmp(argv[1], "delay_lower_write") == 0) delay_lower_write_case();
    else if (strcmp(argv[1], "buddy_limit") == 0) buddy_limit_case();
    else if (strcmp(argv[1], "best_tie") == 0) policy_tie_case(BEST_FIT);
    else if (strcmp(argv[1], "worst_tie") == 0) policy_tie_case(WORST_FIT);
    else {
        fprintf(stderr, "unknown case: %s\n", argv[1]);
        return 2;
    }
    return 0;
}
