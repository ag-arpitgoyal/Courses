/* buddy_size_single: block size = max(32, pow2 >= n+16) for boundary n, one request at a time, dump each */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char *base;
static unsigned char *v_p;

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
    v_p = M((size_t)1ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)1ULL);
    v_p = NULL;
    v_p = M((size_t)15ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)15ULL);
    v_p = NULL;
    v_p = M((size_t)16ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)16ULL);
    v_p = NULL;
    v_p = M((size_t)17ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)17ULL);
    v_p = NULL;
    v_p = M((size_t)32ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)32ULL);
    v_p = NULL;
    v_p = M((size_t)48ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)48ULL);
    v_p = NULL;
    v_p = M((size_t)49ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)49ULL);
    v_p = NULL;
    v_p = M((size_t)112ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)112ULL);
    v_p = NULL;
    v_p = M((size_t)113ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)113ULL);
    v_p = NULL;
    v_p = M((size_t)1008ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)1008ULL);
    v_p = NULL;
    v_p = M((size_t)1009ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)1009ULL);
    v_p = NULL;
    v_p = M((size_t)4080ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)4080ULL);
    v_p = NULL;
    v_p = M((size_t)4081ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)4081ULL);
    v_p = NULL;
    v_p = M((size_t)65520ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)65520ULL);
    v_p = NULL;
    v_p = M((size_t)65521ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)65521ULL);
    v_p = NULL;
    v_p = M((size_t)524272ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)524272ULL);
    v_p = NULL;
    v_p = M((size_t)524273ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)524273ULL);
    v_p = NULL;
    v_p = M((size_t)1048560ULL);
    P("p", v_p);
    my_heap_dump();
    F(v_p, (size_t)1048560ULL);
    v_p = NULL;
    my_heap_dump();
    return 0;
}
