/* buddy_never_grows: region fills exactly with 524288+262144+131072+65536+65536; later requests NULL, no new region */
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
static unsigned char *v_g2;
static unsigned char *v_g3;
static unsigned char *v_g4;
static unsigned char *v_g5;

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
    v_a = M((size_t)524272ULL);
    P("a", v_a);
    v_b = M((size_t)262128ULL);
    P("b", v_b);
    v_c = M((size_t)131056ULL);
    P("c", v_c);
    v_d = M((size_t)65520ULL);
    P("d", v_d);
    v_e = M((size_t)65520ULL);
    P("e", v_e);
    v_f = M((size_t)1ULL);
    P("f", v_f);
    v_g = M((size_t)65520ULL);
    P("g", v_g);
    my_heap_dump();
    F(v_e, (size_t)65520ULL);
    v_e = NULL;
    v_g2 = M((size_t)65521ULL);
    P("g2", v_g2);
    v_g3 = M((size_t)32752ULL);
    P("g3", v_g3);
    v_g4 = M((size_t)32752ULL);
    P("g4", v_g4);
    v_g5 = M((size_t)1ULL);
    P("g5", v_g5);
    my_heap_dump();
    F(v_a, (size_t)524272ULL);
    v_a = NULL;
    F(v_b, (size_t)262128ULL);
    v_b = NULL;
    F(v_c, (size_t)131056ULL);
    v_c = NULL;
    F(v_d, (size_t)65520ULL);
    v_d = NULL;
    F(v_g3, (size_t)32752ULL);
    v_g3 = NULL;
    F(v_g4, (size_t)32752ULL);
    v_g4 = NULL;
    my_heap_dump();
    return 0;
}
