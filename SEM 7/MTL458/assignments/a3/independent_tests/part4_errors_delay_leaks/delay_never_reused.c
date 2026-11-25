/* delay_never_reused: delayed blocks are never reused under any policy, even when an exact fit */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c, *d, *e, *g;
    my_set_delay(2);
    a = my_malloc(10);
    b = my_malloc(10);
    my_free(a);
    c = my_malloc(10);
    my_heap_dump();
    my_set_policy(BEST_FIT);
    d = my_malloc(10);
    my_free(b);
    my_set_policy(WORST_FIT);
    e = my_malloc(10);
    my_set_policy(NEXT_FIT);
    g = my_malloc(10);
    my_heap_dump();
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)g;
    return 0;
}
