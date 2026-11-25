/* Drop-in program: matrices as arrays of row pointers (my_calloc / my_malloc),
 * growing the row-pointer array with my_realloc, products and transposes mod
 * a prime, with the placement policy changed between phases and a delay queue
 * of 64 in the last phase. Everything is freed: leak report shows 0 blocks. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

#define MOD 1000003u

typedef struct {
    size_t r, c;
    uint32_t **row;
} mat;

static uint32_t rs = 4242u;
static uint32_t rnd(void) {
    rs ^= rs << 13;
    rs ^= rs >> 17;
    rs ^= rs << 5;
    return rs;
}

static int mk(mat *m, size_t r, size_t c, int zero) {
    m->r = r;
    m->c = c;
    m->row = my_malloc(r * sizeof(uint32_t *));
    if (!m->row) return 0;
    for (size_t i = 0; i < r; i++) {
        m->row[i] = zero ? my_calloc(c, sizeof(uint32_t)) : my_malloc(c * sizeof(uint32_t));
        if (!m->row[i]) return 0;
        if (!zero)
            for (size_t j = 0; j < c; j++) m->row[i][j] = rnd() % MOD;
    }
    return 1;
}

static void drop(mat *m) {
    for (size_t i = 0; i < m->r; i++) my_free(m->row[i]);
    my_free(m->row);
    m->row = NULL;
}

static int mul(const mat *a, const mat *b, mat *out) {
    if (!mk(out, a->r, b->c, 1)) return 0;
    for (size_t i = 0; i < a->r; i++)
        for (size_t k = 0; k < a->c; k++) {
            uint64_t x = a->row[i][k];
            for (size_t j = 0; j < b->c; j++)
                out->row[i][j] = (uint32_t)((out->row[i][j] + x * b->row[k][j]) % MOD);
        }
    return 1;
}

static int transpose(const mat *a, mat *out) {
    if (!mk(out, a->c, a->r, 1)) return 0;
    for (size_t i = 0; i < a->r; i++)
        for (size_t j = 0; j < a->c; j++) out->row[j][i] = a->row[i][j];
    return 1;
}

static uint64_t chk(const mat *m) {
    uint64_t s = 0;
    for (size_t i = 0; i < m->r; i++)
        for (size_t j = 0; j < m->c; j++) s = (s * 31 + m->row[i][j]) % 0xFFFFFFFBull;
    return s;
}

static int addrows(mat *m, size_t extra) {
    uint32_t **nr = my_realloc(m->row, (m->r + extra) * sizeof(uint32_t *));
    if (!nr) return 0;
    m->row = nr;
    for (size_t i = 0; i < extra; i++) {
        m->row[m->r + i] = my_malloc(m->c * sizeof(uint32_t));
        if (!m->row[m->r + i]) return 0;
        for (size_t j = 0; j < m->c; j++) m->row[m->r + i][j] = (uint32_t)((i + 1) * (j + 3) % MOD);
    }
    m->r += extra;
    return 1;
}

int main(void) {
    static const int pol[4] = {FIRST_FIT, NEXT_FIT, BEST_FIT, WORST_FIT};
    static const char *pname[4] = {"first", "next", "best", "worst"};
    uint64_t prev = 0;
    for (int ph = 0; ph < 4; ph++) {
        my_set_policy(pol[ph]);
        if (ph == 3) my_set_delay(64);
        mat a, b, c, t, d;
        size_t n = 40 + (size_t)ph * 15;
        if (!mk(&a, n, n + 7, 0) || !mk(&b, n + 7, n - 3, 0)) { printf("alloc failed\n"); return 0; }
        if (!mul(&a, &b, &c)) { printf("alloc failed\n"); return 0; }
        if (!transpose(&c, &t)) { printf("alloc failed\n"); return 0; }
        drop(&a);
        if (!addrows(&t, 25)) { printf("alloc failed\n"); return 0; }
        if (!mul(&t, &c, &d)) { printf("alloc failed\n"); return 0; }
        printf("phase %d (%s fit): c %zux%zu chk %llu, t %zux%zu chk %llu, d %zux%zu chk %llu\n", ph, pname[ph],
               c.r, c.c, (unsigned long long)chk(&c), t.r, t.c, (unsigned long long)chk(&t), d.r, d.c,
               (unsigned long long)chk(&d));
        prev = (prev * 7 + chk(&d)) % 1000000007ull;
        drop(&b);
        drop(&c);
        drop(&t);
        drop(&d);
    }
    printf("combined %llu\n", (unsigned long long)prev);
    my_leak_report();
    return 0;
}
