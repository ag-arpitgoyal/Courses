/* p1_many_regions: 21 regions; first fit skips full regions; worst fit ties across regions pick the earliest region; best fit picks the smallest in the last region */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *r0, *r1, *r2, *r3, *r4, *r5, *r6, *r7, *r8, *r9, *r10, *r11, *r12, *r13, *r14, *r15, *r16, *r17, *r18, *r19, *x, *y, *z, *w1, *w2, *w3;

static void chk(const char *name, const char *p, size_t n, unsigned char t)
{
    size_t i;
    for (i = 0; i < n; i++)
        if ((unsigned char)p[i] != t) { printf("%s bad\n", name); return; }
    printf("%s ok\n", name);
}

int main(void)
{
    (void)chk;
    r0 = my_malloc((size_t)65464);
    r1 = my_malloc((size_t)65464);
    r2 = my_malloc((size_t)65464);
    r3 = my_malloc((size_t)65464);
    r4 = my_malloc((size_t)65464);
    r5 = my_malloc((size_t)65464);
    r6 = my_malloc((size_t)65464);
    r7 = my_malloc((size_t)65464);
    r8 = my_malloc((size_t)65464);
    r9 = my_malloc((size_t)65464);
    r10 = my_malloc((size_t)65464);
    r11 = my_malloc((size_t)65464);
    r12 = my_malloc((size_t)65464);
    r13 = my_malloc((size_t)65464);
    r14 = my_malloc((size_t)65464);
    r15 = my_malloc((size_t)65464);
    r16 = my_malloc((size_t)65464);
    r17 = my_malloc((size_t)65464);
    r18 = my_malloc((size_t)65464);
    r19 = my_malloc((size_t)65464);
    my_free(r1);
    my_free(r3);
    my_free(r5);
    my_free(r7);
    my_free(r9);
    my_free(r11);
    my_free(r13);
    my_free(r15);
    my_free(r17);
    my_free(r19);
    x = my_malloc((size_t)10);
    y = my_malloc((size_t)65464);
    z = my_malloc((size_t)70000);
    my_heap_dump();
    my_set_policy(WORST_FIT);
    w1 = my_malloc((size_t)10);
    w2 = my_malloc((size_t)10);
    my_set_policy(BEST_FIT);
    w3 = my_malloc((size_t)10);
    my_heap_dump();
    return 0;
}
