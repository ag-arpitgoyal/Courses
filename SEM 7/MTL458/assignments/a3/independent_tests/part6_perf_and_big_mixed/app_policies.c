/* Drop-in program: the same message-queue workload (variable-size messages
 * created with my_malloc/my_calloc, appended to with my_realloc, consumed and
 * freed in FIFO order with random early drops) is run once under each policy
 * and delay setting. Results are layout independent, so all runs must print
 * the same checksums. Everything is freed: leak report shows 0 blocks. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

#define QCAP 512

typedef struct {
    unsigned char *buf;
    size_t len;
    uint32_t id;
} msg;

static uint32_t rs;
static uint32_t rnd(void) {
    rs ^= rs << 13;
    rs ^= rs >> 17;
    rs ^= rs << 5;
    return rs;
}

static unsigned char pat(uint32_t id, size_t i) { return (unsigned char)(id * 131u + (uint32_t)i * 7u); }

static int run(int policy, int delay, uint64_t *out_sum, long *out_bad) {
    my_set_policy(policy);
    my_set_delay(delay);
    rs = 99991u;
    msg *q = my_calloc(QCAP, sizeof(msg));
    if (!q) return 0;
    size_t head = 0, n = 0;
    uint32_t next_id = 1;
    uint64_t sum = 0;
    long bad = 0;
    for (int step = 0; step < 40000; step++) {
        uint32_t r = rnd() % 100u;
        if (r < 45 && n < QCAP) {
            msg *m = &q[(head + n) % QCAP];
            m->id = next_id++;
            m->len = 1 + rnd() % (r < 5 ? 20000u : 400u);
            m->buf = (r & 1) ? my_calloc(m->len, 1) : my_malloc(m->len);
            if (!m->buf) return 0;
            for (size_t i = 0; i < m->len; i++) m->buf[i] = pat(m->id, i);
            n++;
        } else if (r < 70 && n > 0) {
            msg *m = &q[(head + rnd() % n) % QCAP];
            size_t add = 1 + rnd() % 600u;
            unsigned char *nb = my_realloc(m->buf, m->len + add);
            if (!nb) return 0;
            for (size_t i = m->len; i < m->len + add; i++) nb[i] = pat(m->id, i);
            m->buf = nb;
            m->len += add;
        } else if (r < 80 && n > 0) {
            msg *m = &q[(head + rnd() % n) % QCAP];
            size_t keep = 1 + m->len / 3;
            unsigned char *nb = my_realloc(m->buf, keep);
            if (!nb) return 0;
            m->buf = nb;
            m->len = keep;
        } else if (n > 0) {
            msg *m = &q[head];
            for (size_t i = 0; i < m->len; i++) {
                if (m->buf[i] != pat(m->id, i)) { bad++; break; }
                sum = sum * 1000003u + m->buf[i];
            }
            sum += m->len;
            my_free(m->buf);
            head = (head + 1) % QCAP;
            n--;
        }
    }
    while (n > 0) {
        msg *m = &q[head];
        sum = sum * 31 + m->len + m->buf[m->len - 1];
        my_free(m->buf);
        head = (head + 1) % QCAP;
        n--;
    }
    my_free(q);
    *out_sum = sum;
    *out_bad = bad;
    return 1;
}

int main(void) {
    static const int pol[] = {FIRST_FIT, NEXT_FIT, BEST_FIT, WORST_FIT, NEXT_FIT, BEST_FIT};
    static const int del[] = {0, 0, 5, 64, 1, 0};
    static const char *pname[] = {"first", "next", "best", "worst", "next", "best"};
    for (int i = 0; i < 6; i++) {
        uint64_t s;
        long bad;
        if (!run(pol[i], del[i], &s, &bad)) { printf("alloc failed\n"); return 0; }
        printf("run %d %s fit delay %d: checksum %llu corrupt %ld\n", i, pname[i], del[i], (unsigned long long)s, bad);
    }
    my_set_delay(0);
    my_leak_report();
    return 0;
}
