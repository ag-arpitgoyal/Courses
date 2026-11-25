/* p1_null_and_empty: dump with no region prints nothing; NULL for n==0, n>2^30, SIZE_MAX-ish sizes; my_free(NULL) is a no-op; failed calls create no region */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a;

static void chk(const char *name, const char *p, size_t n, unsigned char t)
{
    size_t i;
    for (i = 0; i < n; i++)
        if ((unsigned char)p[i] != t) { printf("%s bad\n", name); return; }
    printf("%s ok\n", name);
}

int main(void)
{
    (void)chk;
    my_heap_dump();
    printf("malloc(0) %s\n", my_malloc((size_t)0) ? "ptr" : "NULL");
    printf("malloc(1073741825) %s\n", my_malloc((size_t)1073741825) ? "ptr" : "NULL");
    printf("malloc(18446744073709551615) %s\n", my_malloc(SIZE_MAX) ? "ptr" : "NULL");
    printf("malloc(18446744073709551576) %s\n", my_malloc((SIZE_MAX - 39)) ? "ptr" : "NULL");
    printf("malloc(18446744073709551595) %s\n", my_malloc((SIZE_MAX - 20)) ? "ptr" : "NULL");
    printf("malloc(2147483648) %s\n", my_malloc((size_t)2147483648) ? "ptr" : "NULL");
    my_free(NULL);
    my_heap_dump();
    printf("---\n");
    a = my_malloc((size_t)1);
    my_heap_dump();
    my_free(NULL);
    printf("malloc(0) %s\n", my_malloc((size_t)0) ? "ptr" : "NULL");
    printf("malloc(1073741840) %s\n", my_malloc((size_t)1073741840) ? "ptr" : "NULL");
    my_heap_dump();
    return 0;
}
