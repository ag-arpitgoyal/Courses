/* Runs the operations produced by models/heap_model.py against a3.c (see
 * that file for the format). Prints "NULL" whenever a call returns NULL,
 * and checks the data of every block it owns:
 *   - malloc'd blocks are filled with a pattern, checked before free/realloc;
 *   - realloc must keep min(old n, new n) bytes;
 *   - calloc memory must be zero.
 * Any problem prints a line starting with "DATA", which never appears in the
 * model's expected output, so it shows up as a mismatch. */
#include <stdio.h>
#include <string.h>
#include "myalloc.h"

#define MAX_IDS 50000

static unsigned char *ptr[MAX_IDS];
static size_t size[MAX_IDS];
static int owned[MAX_IDS];

static void fill(int id) {
    for (size_t i = 0; i < size[id]; i++) ptr[id][i] = (unsigned char)(id * 7 + i);
}

static void check(int id, size_t upto, const char *when) {
    for (size_t i = 0; i < upto; i++)
        if (ptr[id][i] != (unsigned char)(id * 7 + i)) {
            printf("DATA corrupted: id %d byte %zu (%s)\n", id, i, when);
            return;
        }
}

int main(void) {
    char op[4];
    int id;
    unsigned long a, b;
    while (scanf("%3s", op) == 1) {
        if (!strcmp(op, "m") && scanf("%d %lu", &id, &a) == 2) {
            unsigned char *r = my_malloc(a);
            if (!r) { printf("NULL\n"); continue; }
            ptr[id] = r; size[id] = a; owned[id] = 1;
            fill(id);
        } else if (!strcmp(op, "c") && scanf("%d %lu %lu", &id, &a, &b) == 3) {
            unsigned char *r = my_calloc(a, b);
            if (!r) { printf("NULL\n"); continue; }
            for (size_t i = 0; i < a * b; i++)
                if (r[i]) { printf("DATA calloc memory not zero\n"); break; }
            ptr[id] = r; size[id] = a * b; owned[id] = 1;
            fill(id);
        } else if (!strcmp(op, "r") && scanf("%d %lu", &id, &a) == 2) {
            unsigned char *r = my_realloc(ptr[id], a);
            if (!r) {
                printf("NULL\n");
                if (a == 0) owned[id] = 0;
                continue;
            }
            if (owned[id]) {
                ptr[id] = r;
                check(id, size[id] < a ? size[id] : a, "after realloc");
            }
            ptr[id] = r; size[id] = a; owned[id] = 1;
            fill(id);
        } else if (!strcmp(op, "f") && scanf("%d", &id) == 1) {
            if (owned[id]) check(id, size[id], "before free");
            my_free(ptr[id]);
            owned[id] = 0;
        } else if (!strcmp(op, "x") && scanf("%d", &id) == 1) {
            my_free(ptr[id] + 16);
        } else if (!strcmp(op, "s")) {
            int local;
            my_free(&local);
        } else if (!strcmp(op, "w") && scanf("%d", &id) == 1) {
            ptr[id][0] = 0x58;
        } else if (!strcmp(op, "o") && scanf("%d", &id) == 1) {
            ptr[id][size[id] + 3] = 0;
        } else if (!strcmp(op, "p") && scanf("%d", &id) == 1) {
            my_set_policy(id);
        } else if (!strcmp(op, "k") && scanf("%d", &id) == 1) {
            my_set_delay(id);
        } else if (!strcmp(op, "d")) {
            my_heap_dump();
        } else if (!strcmp(op, "l")) {
            my_leak_report();
        }
    }
    return 0;
}
