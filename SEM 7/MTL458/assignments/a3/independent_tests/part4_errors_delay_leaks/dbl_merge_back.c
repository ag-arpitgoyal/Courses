/* dbl_merge_back: task.md a/b example: after a,b merge, free(a) is double free, free(b) invalid; same via realloc; then c merges */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *q;
    a = my_malloc(10);
    b = my_malloc(10);
    c = my_malloc(10);
    my_free(a);
    my_free(b);
    my_free(a);
    my_free(b);
    q = my_realloc(b, 50);
    printf("q %s\n", q ? "ptr" : "NULL");
    q = my_realloc(a, 50);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_heap_dump();
    my_free(c);
    my_free(c);
    my_free(a);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)q;
    return 0;
}
