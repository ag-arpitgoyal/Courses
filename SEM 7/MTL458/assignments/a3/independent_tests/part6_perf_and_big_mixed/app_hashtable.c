/* Drop-in program: chained hash table with my_calloc'd bucket arrays that
 * double on load > 3/4 (rehash + free old), entries with flexible array keys.
 * Runs under WORST_FIT with a delay queue of 16 (a correct program, so no
 * errors may be printed). Everything is freed: leak report shows 0 blocks. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

struct entry {
    struct entry *next;
    uint32_t hash;
    uint32_t count;
    char key[];
};

static struct entry **tab;
static size_t nb, nent;

static uint32_t rs = 777u;
static uint32_t rnd(void) {
    rs ^= rs << 13;
    rs ^= rs >> 17;
    rs ^= rs << 5;
    return rs;
}

static uint32_t hstr(const char *s) {
    uint32_t h = 2166136261u;
    while (*s) { h ^= (unsigned char)*s++; h *= 16777619u; }
    return h;
}

static int grow(void) {
    size_t nn = nb ? nb * 2 : 8;
    struct entry **nt = my_calloc(nn, sizeof *nt);
    if (!nt) return 0;
    for (size_t i = 0; i < nb; i++) {
        struct entry *e = tab[i];
        while (e) {
            struct entry *nx = e->next;
            size_t b = e->hash & (nn - 1);
            e->next = nt[b];
            nt[b] = e;
            e = nx;
        }
    }
    my_free(tab);
    tab = nt;
    nb = nn;
    return 1;
}

static int add(const char *k) {
    uint32_t h = hstr(k);
    if (nb) {
        for (struct entry *e = tab[h & (nb - 1)]; e; e = e->next)
            if (e->hash == h && strcmp(e->key, k) == 0) { e->count++; return 1; }
    }
    if (nent + 1 > nb * 3 / 4 && !grow()) return 0;
    size_t L = strlen(k);
    struct entry *e = my_malloc(sizeof *e + L + 1);
    if (!e) return 0;
    memcpy(e->key, k, L + 1);
    e->hash = h;
    e->count = 1;
    e->next = tab[h & (nb - 1)];
    tab[h & (nb - 1)] = e;
    nent++;
    return 1;
}

static const char *SYL[] = {"ka", "lo", "mi", "zu", "re", "ta", "no", "shi", "ve", "po", "an", "ex"};

int main(void) {
    my_set_policy(WORST_FIT);
    my_set_delay(16);
    char w[64];
    long total = 0;
    for (int i = 0; i < 60000; i++) {
        int ns = 1 + (int)(rnd() % 4u);
        w[0] = '\0';
        for (int j = 0; j < ns; j++) {
            uint32_t r = rnd() % 144u;
            uint32_t s = 0;
            while ((s + 1) * (s + 1) <= r) s++;
            strcat(w, SYL[11 - s]);
        }
        if (!add(w)) { printf("alloc failed\n"); return 0; }
        total++;
    }
    printf("words %ld distinct %zu buckets %zu\n", total, nent, nb);

    for (int t = 0; t < 10; t++) {
        struct entry *best = NULL;
        for (size_t i = 0; i < nb; i++)
            for (struct entry *e = tab[i]; e; e = e->next) {
                if (e->count & 0x80000000u) continue;
                if (!best || e->count > best->count ||
                    (e->count == best->count && strcmp(e->key, best->key) < 0))
                    best = e;
            }
        printf("top %d: %s %u\n", t + 1, best->key, best->count);
        best->count |= 0x80000000u;
    }
    for (size_t i = 0; i < nb; i++)
        for (struct entry *e = tab[i]; e; e = e->next)
            e->count &= ~0x80000000u;

    size_t removed = 0;
    for (size_t i = 0; i < nb; i++) {
        struct entry **pp = &tab[i];
        while (*pp) {
            struct entry *e = *pp;
            if (e->count == 1) { *pp = e->next; my_free(e); removed++; nent--; }
            else pp = &e->next;
        }
    }
    uint64_t chk = 0;
    size_t longest = 0, used = 0;
    for (size_t i = 0; i < nb; i++) {
        size_t len = 0;
        for (struct entry *e = tab[i]; e; e = e->next) {
            chk += (uint64_t)hstr(e->key) * e->count;
            len++;
        }
        if (len) used++;
        if (len > longest) longest = len;
    }
    printf("removed singletons %zu, remaining %zu, checksum %llu\n", removed, nent, (unsigned long long)chk);
    printf("bucket stats: used %zu longest %zu\n", used, longest);

    for (size_t i = 0; i < nb; i++) {
        struct entry *e = tab[i];
        while (e) { struct entry *nx = e->next; my_free(e); e = nx; }
    }
    my_free(tab);
    my_leak_report();
    my_set_delay(0);
    my_leak_report();
    return 0;
}
