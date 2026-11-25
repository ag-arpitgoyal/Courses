/* buddy_mixed_sequence: longer hand-traced mix of sizes, reuse after partial merges, cascades stopping at allocated buddies */
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
static unsigned char *v_f;
static unsigned char *v_g;
static unsigned char *v_h;
static unsigned char *v_i;
static unsigned char *v_j;
static unsigned char *v_k;

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
    v_a = M((size_t)100ULL);
    P("a", v_a);
    v_b = M((size_t)1000ULL);
    P("b", v_b);
    v_c = M((size_t)20ULL);
    P("c", v_c);
    v_d = M((size_t)300ULL);
    P("d", v_d);
    v_e = M((size_t)2000ULL);
    P("e", v_e);
    v_f = M((size_t)3000ULL);
    P("f", v_f);
    v_g = M((size_t)10ULL);
    P("g", v_g);
    my_heap_dump();
    F(v_a, (size_t)100ULL);
    v_a = NULL;
    F(v_d, (size_t)300ULL);
    v_d = NULL;
    F(v_e, (size_t)2000ULL);
    v_e = NULL;
    F(v_c, (size_t)20ULL);
    v_c = NULL;
    my_heap_dump();
    v_h = M((size_t)40ULL);
    P("h", v_h);
    v_i = M((size_t)200ULL);
    P("i", v_i);
    v_j = M((size_t)100ULL);
    P("j", v_j);
    v_k = M((size_t)700ULL);
    P("k", v_k);
    my_heap_dump();
    F(v_g, (size_t)10ULL);
    v_g = NULL;
    F(v_h, (size_t)40ULL);
    v_h = NULL;
    F(v_j, (size_t)100ULL);
    v_j = NULL;
    my_heap_dump();
    F(v_b, (size_t)1000ULL);
    v_b = NULL;
    F(v_i, (size_t)200ULL);
    v_i = NULL;
    my_heap_dump();
    F(v_f, (size_t)3000ULL);
    v_f = NULL;
    my_heap_dump();
    F(v_k, (size_t)700ULL);
    v_k = NULL;
    my_heap_dump();
    return 0;
}
