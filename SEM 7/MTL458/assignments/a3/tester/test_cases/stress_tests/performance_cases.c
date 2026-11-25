/* Stress workloads for the Part 6 free-block and allocation bounds. */
#include <stdio.h>
#include <string.h>

#include "myalloc.h"

#define N 100000

static void *slots[2 * N];

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    if (strcmp(argv[1], "best") == 0) my_set_policy(BEST_FIT);
    else if (strcmp(argv[1], "worst") == 0) my_set_policy(WORST_FIT);
    else return 2;

    for (size_t i = 0; i < 2 * N; ++i) {
        slots[i] = my_malloc(100);
        if (!slots[i]) {
            fprintf(stderr, "allocation failed during heap setup at %zu\n", i);
            return 3;
        }
    }
    for (size_t i = 0; i < 2 * N; i += 2) my_free(slots[i]);
    for (size_t i = 0; i < N; ++i) {
        void *p = my_malloc(100);
        if (!p) {
            fprintf(stderr, "allocation failed during timed phase at %zu\n", i);
            return 4;
        }
    }
    my_heap_dump();
    return 0;
}
