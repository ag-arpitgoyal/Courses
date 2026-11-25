/* buddy_no_merge_nonbuddy: adjacent equal-size free blocks that are not XOR buddies stay separate; later cascades */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char *base;
static unsigned char *v_p0;
static unsigned char *v_p1;
static unsigned char *v_p2;
static unsigned char *v_p3;
static unsigned char *v_p4;
static unsigned char *v_p5;
static unsigned char *v_p6;
static unsigned char *v_p7;

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
    v_p0 = M((size_t)48ULL);
    P("p0", v_p0);
    v_p1 = M((size_t)48ULL);
    P("p1", v_p1);
    v_p2 = M((size_t)48ULL);
    P("p2", v_p2);
    v_p3 = M((size_t)48ULL);
    P("p3", v_p3);
    v_p4 = M((size_t)48ULL);
    P("p4", v_p4);
    v_p5 = M((size_t)48ULL);
    P("p5", v_p5);
    v_p6 = M((size_t)48ULL);
    P("p6", v_p6);
    v_p7 = M((size_t)48ULL);
    P("p7", v_p7);
    F(v_p1, (size_t)48ULL);
    v_p1 = NULL;
    F(v_p2, (size_t)48ULL);
    v_p2 = NULL;
    my_heap_dump();
    F(v_p0, (size_t)48ULL);
    v_p0 = NULL;
    my_heap_dump();
    F(v_p3, (size_t)48ULL);
    v_p3 = NULL;
    my_heap_dump();
    F(v_p6, (size_t)48ULL);
    v_p6 = NULL;
    F(v_p5, (size_t)48ULL);
    v_p5 = NULL;
    my_heap_dump();
    F(v_p4, (size_t)48ULL);
    v_p4 = NULL;
    my_heap_dump();
    F(v_p7, (size_t)48ULL);
    v_p7 = NULL;
    my_heap_dump();
    return 0;
}
