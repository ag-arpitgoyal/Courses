#include "myalloc.h"
#include <stdio.h>
#include <string.h>

typedef struct Entry {
    char *key;
    int val;
    struct Entry *next;
} Entry;

static unsigned int hash_str(const char *s) {
    unsigned int h = 5381;
    while (*s) {
        h = ((h << 5) + h) ^ (unsigned char)(*s++);
    }
    return h & 31u;
}

int main(void) {
    my_set_policy(NEXT_FIT);
    Entry **buckets = my_calloc(32, sizeof(Entry *));

    /* Insert 40 key-value pairs "key_0" .. "key_39" */
    for (int i = 0; i < 40; i++) {
        char buf[32];
        int len = snprintf(buf, sizeof(buf), "key_%d", i);
        char *k = my_malloc((size_t)len + 1);
        memcpy(k, buf, (size_t)len + 1);

        unsigned int b = hash_str(k);
        Entry *e = my_malloc(sizeof(Entry));
        e->key = k;
        e->val = i * 10;
        e->next = buckets[b];
        buckets[b] = e;
    }

    /* Build a summary string using a growing buffer */
    size_t sb_cap = 16;
    size_t sb_len = 0;
    char *sb = my_malloc(sb_cap);
    sb[0] = '\0';

    int total_val = 0;
    for (int i = 0; i < 40; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "key_%d", i);
        unsigned int b = hash_str(buf);
        for (Entry *e = buckets[b]; e != NULL; e = e->next) {
            if (strcmp(e->key, buf) == 0) {
                total_val += e->val;
                if (i < 5) {
                    char item[32];
                    int ilen = snprintf(item, sizeof(item), "%s=%d;", e->key, e->val);
                    while (sb_len + (size_t)ilen + 1 > sb_cap) {
                        sb_cap *= 2;
                        sb = my_realloc(sb, sb_cap);
                    }
                    memcpy(sb + sb_len, item, (size_t)ilen + 1);
                    sb_len += (size_t)ilen;
                }
                break;
            }
        }
    }

    printf("total_val=%d prefix=%s\n", total_val, sb);

    /* Free everything except one intentional 64-byte sentinel allocation to verify leak report in an app */
    void *sentinel = my_malloc(64);
    (void)sentinel;

    my_free(sb);
    for (int b = 0; b < 32; b++) {
        Entry *e = buckets[b];
        while (e) {
            Entry *next = e->next;
            my_free(e->key);
            my_free(e);
            e = next;
        }
    }
    my_free(buckets);

    /* Wait: sentinel's offset depends on placement, so let's free sentinel too so stdout is 100% deterministic! */
    my_free(sentinel);
    my_leak_report();
    return 0;
}
