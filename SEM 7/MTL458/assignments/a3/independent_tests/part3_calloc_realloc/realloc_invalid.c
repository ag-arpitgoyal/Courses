#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static void fill(void *p, size_t n, unsigned seed) {
    unsigned char *c = p;
    for (size_t i = 0; i < n; i++) c[i] = (unsigned char)(seed + i * 7 + (i >> 8));
}
static int same(const void *p, size_t n, unsigned seed) {
    const unsigned char *c = p;
    for (size_t i = 0; i < n; i++)
        if (c[i] != (unsigned char)(seed + i * 7 + (i >> 8))) return 0;
    return 1;
}

static char sbuf[64];
static int k;
static void R(void *p) {
    printf("realloc %d -> %s\n", k++, p == NULL ? "NULL" : "non-NULL");
}

int main(void) {
    char stackbuf[64];
    memset(stackbuf, 0, sizeof stackbuf);
    unsigned char *a = my_malloc(10);
    unsigned char *b = my_malloc(10);
    unsigned char *c = my_malloc(100);
    unsigned char *d = my_malloc(10);
    unsigned char *e = my_malloc(200);
    unsigned char *f = my_malloc(10);
    fill(c, 100, 1);
    fill(e, 200, 2);
    fill(f, 10, 3);
    my_free(a);
    my_free(b);
    R(my_realloc(a, 20));                         /* start of free block 0:32 */
    R(my_realloc(b, 20));                         /* merged into 0:32 */
    R(my_realloc(b, 0));
    R(my_realloc(a, 0));
    R(my_realloc(a, ((size_t)1 << 30) + 1));
    R(my_realloc(c + 1, 20));
    R(my_realloc(c + 16, 0));
    R(my_realloc(c - 32, 20));
    R(my_realloc(stackbuf, 20));
    R(my_realloc(sbuf, 20));
    R(my_realloc(sbuf, 0));
    R(my_realloc(f + 128, 5));                    /* data start of the split leftover 0:928 */
    my_set_delay(1);
    my_free(d);
    R(my_realloc(d, 50));                         /* delayed block */
    R(my_realloc(d, 0));
    my_heap_dump();
    printf("data %d %d %d\n", same(c, 100, 1), same(e, 200, 2), same(f, 10, 3));
    my_set_delay(0);
    R(my_realloc(d, 5));
    my_heap_dump();
    my_leak_report();
    return 0;
}
