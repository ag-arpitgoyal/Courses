/* Drop-in program: growable int arrays (my_realloc doubling), merge sort with a
 * my_malloc'd scratch buffer, shrinking with my_realloc, and an array of 1000
 * small vectors (my_calloc). Prints only its own results, then a leak report:
 * the very first allocation (48 bytes, at 0:32) is deliberately never freed. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

typedef struct {
    int32_t *data;
    size_t len, cap;
} vec;

static uint32_t rs = 2463534242u;
static uint32_t rnd(void) {
    rs ^= rs << 13;
    rs ^= rs >> 17;
    rs ^= rs << 5;
    return rs;
}

static int push(vec *v, int32_t x) {
    if (v->len == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 4;
        int32_t *nd = my_realloc(v->data, nc * sizeof *nd);
        if (!nd) return 0;
        v->data = nd;
        v->cap = nc;
    }
    v->data[v->len++] = x;
    return 1;
}

static void msort(int32_t *a, int32_t *tmp, size_t n) {
    for (size_t w = 1; w < n; w *= 2) {
        for (size_t lo = 0; lo < n; lo += 2 * w) {
            size_t mid = lo + w < n ? lo + w : n;
            size_t hi = lo + 2 * w < n ? lo + 2 * w : n;
            size_t i = lo, j = mid, k = lo;
            while (i < mid && j < hi) tmp[k++] = a[i] <= a[j] ? a[i++] : a[j++];
            while (i < mid) tmp[k++] = a[i++];
            while (j < hi) tmp[k++] = a[j++];
        }
        memcpy(a, tmp, n * sizeof *a);
    }
}

static int64_t wsum(const int32_t *a, size_t n) {
    int64_t s = 0;
    for (size_t i = 0; i < n; i++) s += (int64_t)a[i] * (int64_t)(i % 7 + 1);
    return s;
}

int main(void) {
    uint64_t *stats = my_malloc(6 * sizeof(uint64_t));
    if (!stats) { printf("stats alloc failed\n"); return 0; }
    memset(stats, 0, 6 * sizeof(uint64_t));

    vec v = {0};
    int64_t sum = 0;
    for (int i = 0; i < 200000; i++) {
        int32_t x = (int32_t)(rnd() % 1000000u) - 500000;
        if (!push(&v, x)) { printf("push failed\n"); return 0; }
        sum += x;
        stats[0]++;
    }
    printf("pushed %zu values, capacity %zu, sum %lld\n", v.len, v.cap, (long long)sum);

    int32_t *tmp = my_malloc(v.len * sizeof *tmp);
    if (!tmp) { printf("tmp alloc failed\n"); return 0; }
    msort(v.data, tmp, v.len);
    my_free(tmp);
    int sorted = 1;
    for (size_t i = 1; i < v.len; i++)
        if (v.data[i - 1] > v.data[i]) sorted = 0;
    printf("sorted=%d min=%d max=%d median=%d wsum=%lld\n", sorted, v.data[0], v.data[v.len - 1],
           v.data[v.len / 2], (long long)wsum(v.data, v.len));

    size_t k = 0;
    for (size_t i = 0; i < v.len; i++)
        if (v.data[i] & 1) v.data[k++] = v.data[i];
    int32_t *shr = my_realloc(v.data, k * sizeof *shr);
    if (!shr) { printf("shrink failed\n"); return 0; }
    v.data = shr;
    v.len = v.cap = k;
    printf("odd values kept %zu, wsum=%lld\n", k, (long long)wsum(v.data, v.len));

    vec *vs = my_calloc(1000, sizeof(vec));
    if (!vs) { printf("calloc failed\n"); return 0; }
    int zero = 1;
    for (int i = 0; i < 1000; i++)
        if (vs[i].data || vs[i].len || vs[i].cap) zero = 0;
    printf("calloc zeroed=%d\n", zero);
    for (int r = 0; r < 60000; r++) {
        uint32_t idx = rnd() % 1000u;
        if (!push(&vs[idx], (int32_t)(rnd() % 1000u))) { printf("push failed\n"); return 0; }
        if (rnd() % 16u == 0 && vs[idx].len > 0) vs[idx].len--;
    }
    int64_t tot = 0, tlen = 0;
    size_t maxlen = 0;
    for (int i = 0; i < 1000; i++) {
        tlen += (int64_t)vs[i].len;
        if (vs[i].len > maxlen) maxlen = vs[i].len;
        tot += wsum(vs[i].data, vs[i].len);
    }
    printf("vectors: total len %lld, max len %zu, checksum %lld\n", (long long)tlen, maxlen, (long long)tot);
    for (int i = 0; i < 1000; i++) my_free(vs[i].data);
    my_free(vs);
    my_free(v.data);
    printf("operations counted: %llu\n", (unsigned long long)stats[0]);
    my_leak_report();
    return 0;
}
