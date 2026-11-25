/* buddy_smallest_fit: smallest fitting free block wins over a larger lower-offset one; too-small blocks skipped */
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
    v_a = M((size_t)496ULL);
    P("a", v_a);
    v_b = M((size_t)16ULL);
    P("b", v_b);
    F(v_a, (size_t)496ULL);
    v_a = NULL;
    my_heap_dump();
    v_c = M((size_t)200ULL);
    P("c", v_c);
    v_d = M((size_t)100ULL);
    P("d", v_d);
    v_e = M((size_t)40ULL);
    P("e", v_e);
    v_f = M((size_t)1ULL);
    P("f", v_f);
    v_g = M((size_t)1ULL);
    P("g", v_g);
    my_heap_dump();
    v_h = M((size_t)300ULL);
    P("h", v_h);
    v_i = M((size_t)200ULL);
    P("i", v_i);
    my_heap_dump();
    return 0;
}
