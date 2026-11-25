/* buddy_example_free: spec example with offsets, then frees: no merge with a different-size buddy, final cascade */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char *base;
static unsigned char *v_a;
static unsigned char *v_b;
static unsigned char *v_c;

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
    v_a = M((size_t)40ULL);
    P("a", v_a);
    v_b = M((size_t)100ULL);
    P("b", v_b);
    v_c = M((size_t)40ULL);
    P("c", v_c);
    F(v_b, (size_t)100ULL);
    v_b = NULL;
    my_heap_dump();
    F(v_a, (size_t)40ULL);
    v_a = NULL;
    my_heap_dump();
    F(v_c, (size_t)40ULL);
    v_c = NULL;
    my_heap_dump();
    return 0;
}
