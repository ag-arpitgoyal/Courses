/* buddy_frag_1024: 1024 x 1024-byte blocks fill the region; free evens: 512KB free yet 2048 request NULL */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char *base;
static unsigned char *v_x;
static unsigned char *v_y1;
static unsigned char *v_y2;
static unsigned char *v_y3;
static unsigned char *v_y4;
static unsigned char *A[32768];
static size_t An[32768];

static unsigned long long off_of(const void *p)
{
    return (unsigned long long)((uintptr_t)p - (uintptr_t)base);
}

static unsigned char pat(const void *p, size_t n)
{
    return (unsigned char)(off_of(p) / 16u * 13u + n * 7u + 0x5au);
}

static void *M(size_t n)
{
    unsigned char *p = my_malloc(n);
    if (p) {
        if (!base)
            base = p;
        if (n <= 1048560)
            memset(p, pat(p, n), n);
    }
    return p;
}

static void P(const char *name, const void *p)
{
    if (p)
        printf("%s = %llu\n", name, off_of(p));
    else
        printf("%s = NULL\n", name);
}

static int bad(const void *p, size_t n)
{
    const unsigned char *q = p;
    unsigned char c = pat(p, n);
    for (size_t i = 0; i < n; i++)
        if (q[i] != c)
            return 1;
    return 0;
}

static void F(void *p, size_t n)
{
    if (!p)
        return;
    if (bad(p, n))
        printf("data corrupted at %llu\n", off_of(p));
    my_free(p);
}

static unsigned acnt;
static unsigned long long asum;
static void AM(unsigned i, size_t n)
{
    A[i] = M(n);
    An[i] = n;
    if (A[i]) {
        acnt++;
        asum += off_of(A[i]);
    }
}

static void AR(unsigned cnt, size_t n)
{
    printf("alloc %u x %llu: %u non-NULL, offset sum %llu\n", cnt, (unsigned long long)n, acnt, asum);
    acnt = 0;
    asum = 0;
}

int main(void)
{
    my_set_policy(BUDDY);
    for (unsigned i = 0; i < 1024; i++)
        AM(i, (size_t)1008ULL);
    AR(1024, (size_t)1008ULL);
    v_x = M((size_t)1ULL);
    P("x", v_x);
    for (long j = 0; j < 512; j++) {
        long i = 0 + j * (2);
        F(A[i], An[i]);
        A[i] = NULL;
    }
    my_heap_dump();
    v_y1 = M((size_t)1009ULL);
    P("y1", v_y1);
    v_y2 = M((size_t)2000ULL);
    P("y2", v_y2);
    v_y3 = M((size_t)1008ULL);
    P("y3", v_y3);
    v_y4 = M((size_t)1ULL);
    P("y4", v_y4);
    my_heap_dump();
    F(v_y3, (size_t)1008ULL);
    v_y3 = NULL;
    F(v_y4, (size_t)1ULL);
    v_y4 = NULL;
    {
        unsigned k = 0;
        for (unsigned i = 0; i < 32768; i++)
            if (A[i] && bad(A[i], An[i]))
                k++;
        printf("check: %u corrupted\n", k);
    }
    for (long j = 0; j < 512; j++) {
        long i = 1 + j * (2);
        F(A[i], An[i]);
        A[i] = NULL;
    }
    my_heap_dump();
    return 0;
}
