/* buddy_fill32_reverse: fill with 32-byte blocks, free in reverse order, refill with n=1, free reverse again */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char *base;
static unsigned char *v_x;
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
    for (long j = 0; j < 32768; j++) {
        long i = 32767 + j * (-1);
        F(A[i], An[i]);
        A[i] = NULL;
    }
    my_heap_dump();
    for (unsigned i = 0; i < 32768; i++)
        AM(i, (size_t)1ULL);
    AR(32768, (size_t)1ULL);
    v_x = M((size_t)1ULL);
    P("x", v_x);
    my_heap_dump();
    for (long j = 0; j < 32768; j++) {
        long i = 32767 + j * (-1);
        F(A[i], An[i]);
        A[i] = NULL;
    }
    my_heap_dump();
    return 0;
}
