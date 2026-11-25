/* Drop-in program: string builder growing with my_realloc (factor 1.5), word
 * strings duplicated with my_malloc into a my_calloc'd pointer array, merge
 * sorted, joined, reversed. Runs under BEST_FIT. The first allocation (the
 * 6-byte banner, at 0:32) is deliberately never freed. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

typedef struct {
    char *s;
    size_t len, cap;
} sbuf;

static uint32_t rs = 1234567u;
static uint32_t rnd(void) {
    rs ^= rs << 13;
    rs ^= rs >> 17;
    rs ^= rs << 5;
    return rs;
}

static int sb_add(sbuf *b, const char *src, size_t n) {
    if (b->len + n + 1 > b->cap) {
        size_t nc = b->cap ? b->cap : 16;
        while (b->len + n + 1 > nc) nc = nc + nc / 2 + 16;
        char *ns = my_realloc(b->s, nc);
        if (!ns) return 0;
        b->s = ns;
        b->cap = nc;
    }
    memcpy(b->s + b->len, src, n);
    b->len += n;
    b->s[b->len] = '\0';
    return 1;
}

static uint64_t fnv(const char *s, size_t n) {
    uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < n; i++) {
        h ^= (unsigned char)s[i];
        h *= 1099511628211ull;
    }
    return h;
}

static char *dup(const char *s) {
    size_t n = strlen(s);
    char *d = my_malloc(n + 1);
    if (d) memcpy(d, s, n + 1);
    return d;
}

static void psort(char **a, char **tmp, size_t n) {
    for (size_t w = 1; w < n; w *= 2) {
        for (size_t lo = 0; lo < n; lo += 2 * w) {
            size_t mid = lo + w < n ? lo + w : n;
            size_t hi = lo + 2 * w < n ? lo + 2 * w : n;
            size_t i = lo, j = mid, k = lo;
            while (i < mid && j < hi) tmp[k++] = strcmp(a[i], a[j]) <= 0 ? a[i++] : a[j++];
            while (i < mid) tmp[k++] = a[i++];
            while (j < hi) tmp[k++] = a[j++];
        }
        memcpy(a, tmp, n * sizeof *a);
    }
}

int main(void) {
    my_set_policy(BEST_FIT);
    char *banner = dup("hello");
    if (!banner) { printf("alloc failed\n"); return 0; }

    sbuf csv = {0};
    char num[32];
    for (int i = 0; i < 30000; i++) {
        int n = snprintf(num, sizeof num, "%d,", i * 37 % 100003);
        if (!sb_add(&csv, num, (size_t)n)) { printf("append failed\n"); return 0; }
    }
    printf("csv length %zu hash %016llx\n", csv.len, (unsigned long long)fnv(csv.s, csv.len));
    printf("csv head %.40s\n", csv.s);
    printf("csv tail %s\n", csv.s + csv.len - 40);

    size_t nw = 5000;
    char **words = my_calloc(nw, sizeof(char *));
    if (!words) { printf("calloc failed\n"); return 0; }
    char w[16];
    for (size_t i = 0; i < nw; i++) {
        int L = 1 + (int)(rnd() % 12u);
        for (int j = 0; j < L; j++) w[j] = (char)('a' + rnd() % 26u);
        w[L] = '\0';
        words[i] = dup(w);
        if (!words[i]) { printf("alloc failed\n"); return 0; }
        if (i % 7 == 3) {
            my_free(words[i - 2]);
            words[i - 2] = dup("zz");
        }
    }
    char **tmp = my_malloc(nw * sizeof(char *));
    if (!tmp) { printf("alloc failed\n"); return 0; }
    psort(words, tmp, nw);
    my_free(tmp);

    sbuf joined = {0};
    for (size_t i = 0; i < nw; i++) {
        if (i) sb_add(&joined, " ", 1);
        sb_add(&joined, words[i], strlen(words[i]));
    }
    printf("joined length %zu hash %016llx\n", joined.len, (unsigned long long)fnv(joined.s, joined.len));
    printf("first words: %s %s %s; last word: %s\n", words[0], words[1], words[2], words[nw - 1]);

    char *rev = my_malloc(joined.len + 1);
    if (!rev) { printf("alloc failed\n"); return 0; }
    for (size_t i = 0; i < joined.len; i++) rev[i] = joined.s[joined.len - 1 - i];
    rev[joined.len] = '\0';
    printf("reversed hash %016llx head %.30s\n", (unsigned long long)fnv(rev, joined.len), rev);

    char *small = my_realloc(joined.s, 21);
    if (!small) { printf("shrink failed\n"); return 0; }
    small[20] = '\0';
    printf("shrunk prefix '%s'\n", small);

    for (size_t i = 0; i < nw; i++) my_free(words[i]);
    my_free(words);
    my_free(rev);
    my_free(small);
    my_free(csv.s);
    printf("%s\n", banner);
    my_leak_report();
    return 0;
}
