/* guard_realloc_shrink: after shrink in place (with and without split) the guard follows the new n; realloc checks guard first */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *p, *p2, *q, *q2, *s, *t, *t2;
    p = my_malloc(100);
    s = my_malloc(10);
    p2 = my_realloc(p, 50);
    printf("p2==p %d\n", p2 == p);
    ((unsigned char *)p2)[53] = 0x00;
    my_free(p2);
    q = my_malloc(300);
    q2 = my_realloc(q, 20);
    printf("q2==q %d\n", q2 == q);
    ((unsigned char *)q2)[20] = 0x01;
    my_free(q2);
    t = my_malloc(100);
    ((unsigned char *)t)[100] = 0x00;
    t2 = my_realloc(t, 50);
    my_free(t2);
    my_heap_dump();
    (void)p; (void)p2; (void)q; (void)q2; (void)s; (void)t; (void)t2;
    return 0;
}
