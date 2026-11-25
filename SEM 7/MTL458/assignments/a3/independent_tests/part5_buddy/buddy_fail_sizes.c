/* buddy_fail_sizes: NULL for n=0, n+16>1048576, 2^30, and when no free block is large enough; heap unchanged */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char *base;
static unsigned char *v_a;
static unsigned char *v_z0;
static unsigned char *v_z1;
static unsigned char *v_z2;
static unsigned char *v_z3;
static unsigned char *v_z4;
static unsigned char *v_z5;
static unsigned char *v_z6;
static unsigned char *v_ok;
static unsigned char *v_z7;
static unsigned char *v_z8;

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

int main(void)
{
    my_set_policy(BUDDY);
    v_a = M((size_t)1ULL);
    P("a", v_a);
    v_z0 = M((size_t)0ULL);
    P("z0", v_z0);
    v_z1 = M((size_t)1048561ULL);
    P("z1", v_z1);
    v_z2 = M((size_t)1048576ULL);
    P("z2", v_z2);
    v_z3 = M((size_t)2000000ULL);
    P("z3", v_z3);
    v_z4 = M((size_t)1073741824ULL);
    P("z4", v_z4);
    v_z5 = M((size_t)1073741825ULL);
    P("z5", v_z5);
    v_z6 = M((size_t)1048560ULL);
    P("z6", v_z6);
    v_ok = M((size_t)524272ULL);
    P("ok", v_ok);
    v_z7 = M((size_t)524273ULL);
    P("z7", v_z7);
    v_z8 = M((size_t)300000ULL);
    P("z8", v_z8);
    my_heap_dump();
    F(v_a, (size_t)1ULL);
    v_a = NULL;
    F(v_ok, (size_t)524272ULL);
    v_ok = NULL;
    my_heap_dump();
    return 0;
}
