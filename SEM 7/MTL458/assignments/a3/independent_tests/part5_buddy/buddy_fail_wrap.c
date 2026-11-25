/* buddy_fail_wrap: huge n where n+16 wraps around in size_t arithmetic (SIZE_MAX-15 etc.) must still be NULL */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char *base;
static unsigned char *v_a;
static unsigned char *v_w1;
static unsigned char *v_w2;
static unsigned char *v_w3;
static unsigned char *v_w4;
static unsigned char *v_w5;
static unsigned char *v_w6;

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
    v_w1 = M((size_t)18446744073709551615ULL);
    P("w1", v_w1);
    v_w2 = M((size_t)18446744073709551600ULL);
    P("w2", v_w2);
    v_w3 = M((size_t)18446744073709551601ULL);
    P("w3", v_w3);
    v_w4 = M((size_t)18446744073709551602ULL);
    P("w4", v_w4);
    v_w5 = M((size_t)9223372036854775808ULL);
    P("w5", v_w5);
    v_w6 = M((size_t)4294967312ULL);
    P("w6", v_w6);
    my_heap_dump();
    F(v_a, (size_t)1ULL);
    v_a = NULL;
    my_heap_dump();
    return 0;
}
