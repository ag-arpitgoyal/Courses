/* buddy_fill32_mod4: fill with 32-byte blocks, free i%4 in {1,2} (adjacent non-buddies, no merge), then i%4==0 (pair merges), then rest */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char *base;
static unsigned char *v_x;
static unsigned char *v_y;
static unsigned char *v_y2;
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
    for (unsigned i = 0; i < 32768; i++)
        AM(i, (size_t)16ULL);
    AR(32768, (size_t)16ULL);
    for (long j = 0; j < 8192; j++) {
        long i = 1 + j * (4);
        F(A[i], An[i]);
        A[i] = NULL;
    }
    for (long j = 0; j < 8192; j++) {
        long i = 2 + j * (4);
        F(A[i], An[i]);
        A[i] = NULL;
    }
    my_heap_dump();
    v_x = M((size_t)17ULL);
    P("x", v_x);
    for (long j = 0; j < 8192; j++) {
        long i = 0 + j * (4);
        F(A[i], An[i]);
        A[i] = NULL;
    }
    my_heap_dump();
    v_y = M((size_t)17ULL);
    P("y", v_y);
    v_y2 = M((size_t)49ULL);
    P("y2", v_y2);
    F(v_y, (size_t)17ULL);
    v_y = NULL;
    {
        unsigned k = 0;
        for (unsigned i = 0; i < 32768; i++)
            if (A[i] && bad(A[i], An[i]))
                k++;
        printf("check: %u corrupted\n", k);
    }
    for (long j = 0; j < 8192; j++) {
        long i = 3 + j * (4);
        F(A[i], An[i]);
        A[i] = NULL;
    }
    my_heap_dump();
    return 0;
}
