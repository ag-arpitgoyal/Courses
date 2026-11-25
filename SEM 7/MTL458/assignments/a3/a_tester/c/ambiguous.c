/* Probes for points the PDF leaves open. The tester does not mark these as
 * pass or fail: it reports which interpretation your code takes, so you can
 * compare with what the TA says. */
#include <stdio.h>
#include <string.h>
#include "myalloc.h"

/* Is region 0 always 65536, even when the first request is bigger? */
static void first_request_large(void) {
    my_malloc(100000);
    my_heap_dump();
}

/* Does a request that returns NULL (n = 0) create region 0? */
static void invalid_request_creates_region(void) {
    my_malloc(0);
    my_heap_dump();
}

/* Does growing/shrinking in place move the next-fit position? */
static void inplace_realloc_next_fit(void) {
    char *a = my_malloc(10);           /* 0:32 */
    void *b = my_malloc(10);           /* 0:160 */
    void *c = my_malloc(10);           /* 0:288 */
    void *d = my_malloc(10);           /* 0:416; position 0:544 */
    (void)d;
    my_free(b);
    my_free(c);                        /* 0:160 free 256 */
    my_realloc(a, 100);                /* grows in place to 144, leaves 0:176 F 240 */
    my_set_policy(NEXT_FIT);
    my_malloc(10);                     /* 0:176 if the position moved to 0:176, else 0:544 */
    my_heap_dump();
}

/* Are guard bytes checked when realloc gets n > 2^30? */
static void guard_check_huge_realloc(void) {
    char *a = my_malloc(10);
    a[10] = 0;
    my_realloc(a, (1UL << 30) + 1);
    printf("--\n");
}

/* What does my_set_delay do with k > 64 or k < 0? */
static void delay_out_of_range(void) {
    my_set_delay(2);
    void *p[3];
    for (int i = 0; i < 3; i++) p[i] = my_malloc(10);
    my_free(p[0]);
    my_free(p[1]);                     /* two delayed */
    my_set_delay(-1);                  /* ignored: still 2 delayed; clamped to 0: none */
    my_heap_dump();
}

int main(int argc, char **argv) {
    if (argc < 2) return 2;
    const char *t = argv[1];
    if (!strcmp(t, "first_request_large")) first_request_large();
    else if (!strcmp(t, "invalid_request_creates_region")) invalid_request_creates_region();
    else if (!strcmp(t, "inplace_realloc_next_fit")) inplace_realloc_next_fit();
    else if (!strcmp(t, "guard_check_huge_realloc")) guard_check_huge_realloc();
    else if (!strcmp(t, "delay_out_of_range")) delay_out_of_range();
    else return 2;
    return 0;
}
