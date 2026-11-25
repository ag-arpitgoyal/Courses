/* buddy_rand_small: 20k ops, 600 slots, mostly small sizes plus boundaries and n=0 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

#define NSLOTS 600
#define NOPS 20000
#define REPORT 500
#define DUMP 5000

struct cls { unsigned cum; int type; unsigned long long lo, hi; };
static const struct cls classes[] = {
    { 400, 0, 1ULL, 64ULL },
    { 700, 0, 65ULL, 1024ULL },
    { 850, 0, 1025ULL, 8192ULL },
    { 950, 1, 5ULL, 14ULL },
    { 990, 0, 8193ULL, 65536ULL },
    { 1000, 0, 0ULL, 0ULL },
};

static unsigned char *slot[NSLOTS];
static size_t slotn[NSLOTS];
static unsigned char slottag[NSLOTS];
static unsigned char *base;
static uint64_t st = 1ULL;

static uint64_t rnd(void)
{
    uint64_t z = (st += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

static size_t gen_size(void)
{
    unsigned r = (unsigned)(rnd() % 1000);
    unsigned c = 0;
    while (r >= classes[c].cum)
        c++;
    unsigned long long v = classes[c].lo + rnd() % (classes[c].hi - classes[c].lo + 1);
    if (classes[c].type == 0)
        return (size_t)v;
    return ((size_t)1 << v) - 17 + (size_t)(rnd() % 3);
}

static void mark(unsigned char *p, size_t n, unsigned char t)
{
    if (n <= 64) {
        memset(p, t, n);
        return;
    }
    memset(p, t, 32);
    memset(p + n - 32, t ^ 0x55, 32);
}

static int bad(const unsigned char *p, size_t n, unsigned char t)
{
    if (n <= 64) {
        for (size_t i = 0; i < n; i++)
            if (p[i] != t)
                return 1;
        return 0;
    }
    for (size_t i = 0; i < 32; i++)
        if (p[i] != t || p[n - 32 + i] != (unsigned char)(t ^ 0x55))
            return 1;
    return 0;
}

int main(void)
{
    uint64_t h = 1469598103934665603ULL;
    unsigned live = 0, nulls = 0, corrupt = 0;

    my_set_policy(BUDDY);
    for (long i = 0; i < NOPS; i++) {
        unsigned s = (unsigned)(rnd() % NSLOTS);
        if (slot[s]) {
            if (bad(slot[s], slotn[s], slottag[s]))
                corrupt++;
            my_free(slot[s]);
            slot[s] = NULL;
            live--;
        }
        else {
            size_t n = gen_size();
            unsigned char *p = my_malloc(n);
            if (p) {
                if (!base)
                    base = p;
                slot[s] = p;
                slotn[s] = n;
                slottag[s] = (unsigned char)(s * 31u + (unsigned)i);
                mark(p, n, slottag[s]);
                live++;
                h = (h ^ ((uint64_t)((uintptr_t)p - (uintptr_t)base) + 1)) * 1099511628211ULL;
            } else {
                nulls++;
                h *= 1099511628211ULL;
            }
        }
        if ((i + 1) % REPORT == 0)
            printf("op %ld live %u nulls %u hash %016llx\n", i + 1, live, nulls, (unsigned long long)h);
        if ((i + 1) % DUMP == 0)
            my_heap_dump();
    }
    printf("corrupt %u\n", corrupt);
    my_heap_dump();
    for (unsigned s = 0; s < NSLOTS; s++) {
        if (slot[s]) {
            if (bad(slot[s], slotn[s], slottag[s]))
                corrupt++;
            my_free(slot[s]);
        }
    }
    printf("corrupt %u\n", corrupt);
    my_heap_dump();
    return 0;
}
