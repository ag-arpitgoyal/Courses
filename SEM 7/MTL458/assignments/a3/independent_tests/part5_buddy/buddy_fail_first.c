/* buddy_fail_first: failed first requests (n=0, n=1048561) create no region: dump still prints nothing */
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

int main(void)
{
    my_set_policy(BUDDY);
    v_a = M((size_t)0ULL);
    P("a", v_a);
    my_heap_dump();
    v_b = M((size_t)1048561ULL);
    P("b", v_b);
    my_heap_dump();
    printf("--\n");
    v_c = M((size_t)5ULL);
    P("c", v_c);
    my_heap_dump();
    return 0;
}
