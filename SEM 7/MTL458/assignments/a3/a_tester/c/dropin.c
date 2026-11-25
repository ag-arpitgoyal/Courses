/* An ordinary program that uses the allocator in place of malloc, like the
 * "drop-in" tests the PDF mentions. It checks its own results and prints
 * "DROPIN OK" if everything is right, then calls my_leak_report (which must
 * report 0 blocks, since the program frees everything).
 *
 * usage: dropin <policy number> */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myalloc.h"

#define malloc  my_malloc
#define calloc  my_calloc
#define realloc my_realloc
#define free    my_free

static int failures = 0;
static int misaligned = 0;
static int buddy_mode = 0;

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } } while (0)

static void note_alignment(void *p) {
    if (p && ((uintptr_t)p % 16) != 0) misaligned++;
}

/* A growing array of ints, resized with realloc like a typical vector. */
static void vector_test(void) {
    int n = 200000;
    int *v = NULL;
    int cap = 0;
    for (int i = 0; i < n; i++) {
        if (i == cap) {
            cap = cap ? cap * 2 : 4;
            int *nv = realloc(v, (size_t)cap * sizeof(int));
            CHECK(nv != NULL, "vector realloc returned NULL");
            if (!nv) return;
            note_alignment(nv);
            v = nv;
        }
        v[i] = i * 3 + 1;
    }
    long long sum = 0;
    for (int i = 0; i < n; i++) sum += v[i];
    CHECK(sum == 3LL * n * (n - 1) / 2 + n, "vector contents wrong after reallocs");
    v = realloc(v, 10 * sizeof(int));                 /* shrink keeps the start */
    CHECK(v && v[9] == 28, "vector shrink lost data");
    free(v);
}

/* A singly linked list built and torn down in mixed order. */
typedef struct ListNode { struct ListNode *next; long value; char tag[24]; } ListNode;

static void list_test(void) {
    int n = buddy_mode ? 2000 : 50000;
    ListNode *head = NULL;
    for (int i = 0; i < n; i++) {
        ListNode *node = malloc(sizeof(ListNode));
        CHECK(node != NULL, "list malloc returned NULL");
        if (!node) return;
        note_alignment(node);
        node->value = i;
        snprintf(node->tag, sizeof node->tag, "node-%d", i);
        node->next = head;
        head = node;
    }
    /* remove every third node */
    ListNode **link = &head;
    int index = n - 1;
    while (*link) {
        ListNode *cur = *link;
        if (index % 3 == 0) { *link = cur->next; free(cur); }
        else link = &cur->next;
        index--;
    }
    long long sum = 0;
    int count = 0;
    char expect[24];
    int tags_ok = 1;
    for (ListNode *p = head; p; p = p->next) {
        sum += p->value;
        count++;
        snprintf(expect, sizeof expect, "node-%ld", p->value);
        if (strcmp(expect, p->tag) != 0) tags_ok = 0;
    }
    long long want = 0;
    int want_count = 0;
    for (int i = 0; i < n; i++) if (i % 3 != 0) { want += i; want_count++; }
    CHECK(count == want_count && sum == want, "list contents wrong");
    CHECK(tags_ok, "list node strings corrupted");
    while (head) { ListNode *next = head->next; free(head); head = next; }
}

/* A hash table of strings with chaining, using calloc for the buckets. */
typedef struct Entry { struct Entry *next; char *key; int count; } Entry;

static unsigned hash(const char *s) {
    unsigned h = 2166136261u;
    while (*s) h = (h ^ (unsigned char)*s++) * 16777619u;
    return h;
}

static void hash_test(void) {
    int buckets = 4096;
    int words = 60000;
    Entry **table = calloc((size_t)buckets, sizeof(Entry *));
    CHECK(table != NULL, "calloc of hash buckets returned NULL");
    if (!table) return;
    int zero = 1;
    for (int i = 0; i < buckets; i++) if (table[i]) zero = 0;
    CHECK(zero, "calloc memory not zeroed");
    char word[32];
    for (int i = 0; i < words; i++) {
        snprintf(word, sizeof word, "w%d", (i * 7919) % (words / 3));
        unsigned b = hash(word) % (unsigned)buckets;
        Entry *e = table[b];
        while (e && strcmp(e->key, word) != 0) e = e->next;
        if (!e) {
            e = malloc(sizeof(Entry));
            CHECK(e != NULL, "hash entry malloc returned NULL");
            if (!e) return;
            e->key = malloc(strlen(word) + 1);
            CHECK(e->key != NULL, "hash key malloc returned NULL");
            if (!e->key) return;
            strcpy(e->key, word);
            e->count = 0;
            e->next = table[b];
            table[b] = e;
        }
        e->count++;
    }
    long total = 0;
    int distinct = 0;
    for (int i = 0; i < buckets; i++)
        for (Entry *e = table[i]; e; e = e->next) { total += e->count; distinct++; }
    CHECK(total == words, "hash counts wrong");
    CHECK(distinct == words / 3, "hash distinct keys wrong");
    for (int i = 0; i < buckets; i++) {
        Entry *e = table[i];
        while (e) { Entry *next = e->next; free(e->key); free(e); e = next; }
    }
    free(table);
}

/* A string builder that grows with realloc and is checked at the end. */
static void string_test(void) {
    int pieces = 20000;
    char *s = calloc(1, 1);
    size_t len = 0;
    CHECK(s != NULL, "string calloc returned NULL");
    if (!s) return;
    char piece[16];
    for (int i = 0; i < pieces; i++) {
        int k = snprintf(piece, sizeof piece, "%d,", i);
        char *ns = realloc(s, len + (size_t)k + 1);
        CHECK(ns != NULL, "string realloc returned NULL");
        if (!ns) { free(s); return; }
        s = ns;
        memcpy(s + len, piece, (size_t)k + 1);
        len += (size_t)k;
    }
    /* verify by re-reading the numbers */
    int expect = 0, ok = 1;
    for (char *p = s; *p; ) {
        int v = (int)strtol(p, &p, 10);
        if (v != expect++ || *p != ',') { ok = 0; break; }
        p++;
    }
    CHECK(ok && expect == pieces, "string builder contents wrong");
    free(s);
}

/* Interleaved lifetimes: many live blocks of random sizes with patterns. */
static void churn_test(void) {
    enum { SLOTS = 4000 };
    static unsigned char *slot[SLOTS];
    static size_t size[SLOTS];
    unsigned rng = 12345;
    int rounds = buddy_mode ? 20000 : 200000;
    size_t max_size = buddy_mode ? 200 : 3000;
    for (int r = 0; r < rounds; r++) {
        rng = rng * 1103515245u + 12345u;
        int i = (int)((rng >> 8) % SLOTS);
        if (slot[i]) {
            for (size_t j = 0; j < size[i]; j++)
                if (slot[i][j] != (unsigned char)(i + j)) { CHECK(0, "churn block corrupted"); r = rounds; break; }
            free(slot[i]);
            slot[i] = NULL;
        } else {
            size[i] = 1 + (rng >> 4) % max_size;
            slot[i] = malloc(size[i]);
            if (!slot[i]) { if (!buddy_mode) CHECK(0, "churn malloc returned NULL"); continue; }
            note_alignment(slot[i]);
            for (size_t j = 0; j < size[i]; j++) slot[i][j] = (unsigned char)(i + j);
        }
    }
    for (int i = 0; i < SLOTS; i++) free(slot[i]);
}

int main(int argc, char **argv) {
    int policy = argc > 1 ? atoi(argv[1]) : FIRST_FIT;
    buddy_mode = (policy == BUDDY);
    my_set_policy(policy);
    if (buddy_mode) {
        /* The PDF says buddy tests only use malloc, free and the dump. */
        list_test();
        churn_test();
    } else {
        vector_test();
        list_test();
        hash_test();
        string_test();
        churn_test();
    }
    if (failures == 0) printf("DROPIN OK\n");
    printf("MISALIGNED %d\n", misaligned);
    if (!buddy_mode) my_leak_report();
    return 0;
}
