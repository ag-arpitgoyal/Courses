/* buddy_frag_quarters: NULL when total free space suffices but no single block does; works after buddy merge */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char *base;
static unsigned char *v_a0;
static unsigned char *v_a1;
static unsigned char *v_a2;
static unsigned char *v_a3;
static unsigned char *v_x;
static unsigned char *v_y1;
static unsigned char *v_y2;
static unsigned char *v_y3;
static unsigned char *v_z;

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
    v_a0 = M((size_t)262128ULL);
    P("a0", v_a0);
    v_a1 = M((size_t)262128ULL);
    P("a1", v_a1);
    v_a2 = M((size_t)262128ULL);
    P("a2", v_a2);
    v_a3 = M((size_t)262128ULL);
    P("a3", v_a3);
    v_x = M((size_t)1ULL);
    P("x", v_x);
    F(v_a0, (size_t)262128ULL);
    v_a0 = NULL;
    F(v_a2, (size_t)262128ULL);
    v_a2 = NULL;
    my_heap_dump();
    v_y1 = M((size_t)262129ULL);
    P("y1", v_y1);
    v_y2 = M((size_t)300000ULL);
    P("y2", v_y2);
    F(v_a1, (size_t)262128ULL);
    v_a1 = NULL;
    my_heap_dump();
    v_y3 = M((size_t)262129ULL);
    P("y3", v_y3);
    my_heap_dump();
    v_z = M((size_t)262128ULL);
    P("z", v_z);
    my_heap_dump();
    F(v_y3, (size_t)262129ULL);
    v_y3 = NULL;
    F(v_z, (size_t)262128ULL);
    v_z = NULL;
    F(v_a3, (size_t)262128ULL);
    v_a3 = NULL;
    my_heap_dump();
    return 0;
}
