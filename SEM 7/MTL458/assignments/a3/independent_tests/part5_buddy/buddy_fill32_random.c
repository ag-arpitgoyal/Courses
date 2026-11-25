/* buddy_fill32_random: fill with 32-byte blocks, free a random half, refill with n=1, free everything in random order (intermediate dumps model-derived; final dump hand-derived) */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char *base;
static unsigned char *v_x;
static unsigned char *A[32768];
static size_t An[32768];
static unsigned perm[32768];
static uint64_t st;
static uint64_t rnd(void)
{
    uint64_t z = (st += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

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
    st = 12345ULL;
    for (unsigned i = 0; i < 32768; i++)
        perm[i] = i;
    for (unsigned i = 32768 - 1; i > 0; i--) {
        unsigned j = (unsigned)(rnd() % (i + 1));
        unsigned t = perm[i];
        perm[i] = perm[j];
        perm[j] = t;
    }
    for (unsigned j = 0; j < 16384; j++) {
        F(A[perm[j]], An[perm[j]]);
        A[perm[j]] = NULL;
    }
    my_heap_dump();
    for (unsigned j = 0; j < 16384; j++)
        AM(perm[j], (size_t)1ULL);
    AR(16384, (size_t)1ULL);
    v_x = M((size_t)1ULL);
    P("x", v_x);
    my_heap_dump();
    {
        unsigned k = 0;
        for (unsigned i = 0; i < 32768; i++)
            if (A[i] && bad(A[i], An[i]))
                k++;
        printf("check: %u corrupted\n", k);
    }
    st = 999ULL;
    for (unsigned i = 0; i < 32768; i++)
        perm[i] = i;
    for (unsigned i = 32768 - 1; i > 0; i--) {
        unsigned j = (unsigned)(rnd() % (i + 1));
        unsigned t = perm[i];
        perm[i] = perm[j];
        perm[j] = t;
    }
    for (unsigned j = 0; j < 16384; j++) {
        F(A[perm[j]], An[perm[j]]);
        A[perm[j]] = NULL;
    }
    my_heap_dump();
    for (unsigned j = 16384; j < 32768; j++) {
        F(A[perm[j]], An[perm[j]]);
        A[perm[j]] = NULL;
    }
    my_heap_dump();
    return 0;
}
