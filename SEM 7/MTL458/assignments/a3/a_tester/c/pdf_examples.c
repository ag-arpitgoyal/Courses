/* The worked examples from the assignment PDF, one per command-line name.
 * Each runs in a fresh process, so every test starts with an empty heap. */
#include <stdio.h>
#include <string.h>
#include "myalloc.h"

static void part1(void) {
    void *a = my_malloc(10), *b = my_malloc(100), *c = my_malloc(10);
    (void)c;
    my_heap_dump();
    my_free(b);
    my_free(a);
    void *d = my_malloc(100000);
    (void)d;
    my_heap_dump();
}

/* Part 2 setup; with policy >= 0 it then sets that policy and asks for 100 bytes. */
static void part2(int policy) {
    void *a = my_malloc(248), *x1 = my_malloc(10), *b = my_malloc(120), *x2 = my_malloc(10);
    void *c = my_malloc(360), *x3 = my_malloc(10);
    (void)x1; (void)x2; (void)x3;
    my_free(b);
    void *y = my_malloc(120);
    my_free(y);
    my_free(a);
    my_free(c);
    if (policy >= 0) {
        my_set_policy(policy);
        my_malloc(100);
    }
    my_heap_dump();
}

static void part3(void) {
    char *p = my_malloc(100), *q = my_malloc(200), *r = my_malloc(10);
    (void)r;
    my_free(q);
    p = my_realloc(p, 250);
    my_heap_dump();
    p = my_realloc(p, 20);
    my_heap_dump();
}

static void part4_delay(void) {
    my_set_delay(2);
    char *a = my_malloc(10), *b = my_malloc(10), *c = my_malloc(10), *d = my_malloc(10);
    my_free(a);
    my_free(b);
    b[3] = 'X';
    my_free(c);
    my_free(d);
    my_heap_dump();
    my_free(a);
    my_free(b);
}

static void part4_leak(void) {
    void *a = my_malloc(100), *b = my_malloc(10), *c = my_malloc(10);
    (void)a; (void)c;
    my_free(b);
    my_leak_report();
}

static void part5(void) {
    my_set_policy(BUDDY);
    my_malloc(40);
    my_malloc(100);
    my_malloc(40);
    my_heap_dump();
}

int main(int argc, char **argv) {
    if (argc < 2) return 2;
    const char *t = argv[1];
    if (!strcmp(t, "part1")) part1();
    else if (!strcmp(t, "part2")) part2(-1);
    else if (!strcmp(t, "part2_first_fit")) part2(FIRST_FIT);
    else if (!strcmp(t, "part2_next_fit")) part2(NEXT_FIT);
    else if (!strcmp(t, "part2_best_fit")) part2(BEST_FIT);
    else if (!strcmp(t, "part2_worst_fit")) part2(WORST_FIT);
    else if (!strcmp(t, "part3")) part3();
    else if (!strcmp(t, "part4_delay")) part4_delay();
    else if (!strcmp(t, "part4_leak")) part4_leak();
    else if (!strcmp(t, "part5")) part5();
    else return 2;
    return 0;
}
