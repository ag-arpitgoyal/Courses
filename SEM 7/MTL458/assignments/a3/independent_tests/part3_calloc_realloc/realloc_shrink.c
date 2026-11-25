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

int main(void) {
    unsigned char *p1 = my_malloc(500);
    unsigned char *p2 = my_malloc(10);
    unsigned char *p3 = my_malloc(300);
    unsigned char *p4 = my_malloc(10);
    unsigned char *p5 = my_malloc(1000);
    unsigned char *p6 = my_malloc(10);
    unsigned char *p7 = my_malloc(600);
    fill(p1, 500, 1);
    fill(p3, 300, 3);
    fill(p5, 1000, 5);
    fill(p7, 600, 7);
    my_heap_dump();

    /* split, next block allocated */
    printf("same %d\n", my_realloc(p1, 100) == p1);
    /* split, tail coalesces with the following free block */
    my_free(p4);
    printf("same %d\n", my_realloc(p3, 100) == p3);
    /* S - B = 112: no split; then same block size, smaller n */
    printf("same %d\n", my_realloc(p5, 888) == p5);
    printf("same %d\n", my_realloc(p5, 887) == p5);
    my_heap_dump();

    /* S - B = 128: split; next block is delayed so no coalescing */
    my_set_delay(1);
    my_free(p6);
    printf("same %d\n", my_realloc(p5, 872) == p5);
    my_heap_dump();
    my_set_delay(0);

    /* tail coalesces with the free tail of the region */
    printf("same %d\n", my_realloc(p7, 100) == p7);
    /* same size */
    printf("same %d\n", my_realloc(p1, 100) == p1);
    /* block before is free, no split (S - B = 16) */
    my_free(p2);
    printf("same %d\n", my_realloc(p3, 50) == p3);

    /* last block of a region: no next block */
    unsigned char *p8 = my_malloc(65464);
    fill(p8, 65464, 8);
    printf("same %d\n", my_realloc(p8, 65400) == p8);
    printf("same %d\n", my_realloc(p8, 1000) == p8);
    my_heap_dump();
    printf("data %d %d %d %d %d\n", same(p1, 100, 1), same(p3, 50, 3), same(p5, 872, 5),
           same(p7, 100, 7), same(p8, 1000, 8));
    my_leak_report();
    return 0;
}
