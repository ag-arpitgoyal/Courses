/* buddy_buddy_size_mismatch: a free block at the buddy offset but of a different size must not be merged */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char *base;
static unsigned char *v_a;
static unsigned char *v_b;
static unsigned char *v_c;
static unsigned char *v_d;
static unsigned char *v_e;

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
    v_b = M((size_t)1ULL);
    P("b", v_b);
    v_c = M((size_t)40ULL);
    P("c", v_c);
    v_d = M((size_t)100ULL);
    P("d", v_d);
    v_e = M((size_t)200ULL);
    P("e", v_e);
    F(v_c, (size_t)40ULL);
    v_c = NULL;
    my_heap_dump();
    F(v_a, (size_t)1ULL);
    v_a = NULL;
    my_heap_dump();
    F(v_e, (size_t)200ULL);
    v_e = NULL;
    my_heap_dump();
    F(v_b, (size_t)1ULL);
    v_b = NULL;
    my_heap_dump();
    F(v_d, (size_t)100ULL);
    v_d = NULL;
    my_heap_dump();
    return 0;
}
