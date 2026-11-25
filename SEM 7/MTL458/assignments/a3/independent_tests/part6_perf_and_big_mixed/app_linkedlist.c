/* Drop-in program: doubly linked list with per-node payloads under NEXT_FIT.
 * Insert at head/tail, delete multiples of 3, reverse, merge sort the list,
 * verify payload integrity, free everything. The first three allocations
 * (100, 10 and 1000 bytes; at 0:32, 0:176, 0:304 in a fresh heap) are
 * deliberately leaked; the leak report lists exactly those. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

struct node {
    int32_t key;
    uint32_t plen;
    unsigned char *payload;
    struct node *prev, *next;
};

static uint32_t rs = 88172645u;
static uint32_t rnd(void) {
    rs ^= rs << 13;
    rs ^= rs >> 17;
    rs ^= rs << 5;
    return rs;
}

static struct node *head, *tail;
static long count;

static struct node *mk(int32_t key) {
    struct node *n = my_malloc(sizeof *n);
    if (!n) return NULL;
    n->key = key;
    n->plen = 1 + (uint32_t)(key % 300);
    n->payload = my_malloc(n->plen);
    if (!n->payload) return NULL;
    memset(n->payload, (unsigned char)(key * 7 + 3), n->plen);
    n->prev = n->next = NULL;
    return n;
}

static int intact(const struct node *n) {
    for (uint32_t i = 0; i < n->plen; i++)
        if (n->payload[i] != (unsigned char)(n->key * 7 + 3)) return 0;
    return 1;
}

static void unlink_free(struct node *n) {
    if (n->prev) n->prev->next = n->next; else head = n->next;
    if (n->next) n->next->prev = n->prev; else tail = n->prev;
    my_free(n->payload);
    my_free(n);
    count--;
}

static struct node *merge(struct node *a, struct node *b) {
    struct node dummy, *t = &dummy;
    while (a && b) {
        if (a->key <= b->key) { t->next = a; a = a->next; }
        else { t->next = b; b = b->next; }
        t = t->next;
    }
    t->next = a ? a : b;
    return dummy.next;
}

static struct node *msort(struct node *h, long n) {
    if (n <= 1) { if (h) h->next = NULL; return h; }
    long half = n / 2;
    struct node *mid = h;
    for (long i = 0; i < half; i++) mid = mid->next;
    return merge(msort(h, half), msort(mid, n - half));
}

static void report(const char *what) {
    long n = 0, bad = 0;
    int64_t sum = 0, ws = 0;
    for (struct node *p = head; p; p = p->next) {
        sum += p->key;
        ws += (int64_t)p->key * (n % 13 + 1);
        if (!intact(p)) bad++;
        n++;
    }
    printf("%s: count=%ld walked=%ld sum=%lld wsum=%lld corrupt=%ld first:", what, count, n,
           (long long)sum, (long long)ws, bad);
    int i = 0;
    for (struct node *p = head; p && i < 8; p = p->next, i++) printf(" %d", p->key);
    printf("\n");
}

int main(void) {
    char *config = my_malloc(100);
    char *flag = my_malloc(10);
    char *buffer = my_malloc(1000);
    if (!config || !flag || !buffer) { printf("alloc failed\n"); return 0; }
    strcpy(config, "mode=list");
    strcpy(flag, "on");
    memset(buffer, 'b', 1000);

    my_set_policy(NEXT_FIT);
    for (int i = 0; i < 12000; i++) {
        struct node *n = mk((int32_t)(rnd() % 100000u));
        if (!n) { printf("alloc failed\n"); return 0; }
        if (rnd() & 1) {
            n->next = head;
            if (head) head->prev = n; else tail = n;
            head = n;
        } else {
            n->prev = tail;
            if (tail) tail->next = n; else head = n;
            tail = n;
        }
        count++;
        if (i % 5 == 4) {
            struct node *v = (rnd() & 1) ? head : tail;
            unlink_free(v);
        }
    }
    report("built");

    for (struct node *p = head, *nx; p; p = nx) {
        nx = p->next;
        if (p->key % 3 == 0) unlink_free(p);
    }
    report("no multiples of 3");

    for (struct node *p = head, *nx; p; p = nx) {
        nx = p->next;
        p->next = p->prev;
        p->prev = nx;
    }
    struct node *t = head; head = tail; tail = t;
    report("reversed");

    head = msort(head, count);
    struct node *prev = NULL;
    for (struct node *p = head; p; p = p->next) { p->prev = prev; prev = p; }
    tail = prev;
    report("sorted");

    long grown = 0;
    for (struct node *p = head; p; p = p->next) {
        if (p->key % 10 == 1) {
            unsigned char *np = my_realloc(p->payload, p->plen + 500);
            if (!np) { printf("realloc failed\n"); return 0; }
            memset(np + p->plen, (unsigned char)(p->key * 7 + 3), 500);
            p->payload = np;
            p->plen += 500;
            grown++;
        }
    }
    printf("grown payloads: %ld\n", grown);
    report("after growth");

    while (head) unlink_free(head);
    report("emptied");
    printf("%s %s %c\n", config, flag, buffer[999]);
    my_leak_report();
    return 0;
}
