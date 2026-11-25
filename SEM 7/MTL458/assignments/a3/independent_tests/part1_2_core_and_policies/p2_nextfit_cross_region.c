/* p2_nextfit_cross_region: next fit across regions: position in region 1 beats an earlier free region 0; wrap back to region 0; position at region end moves on to region 1 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static char *a, *b, *c, *d, *e, *f, *g;

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
    a = my_malloc((size_t)65464);
    b = my_malloc((size_t)10);
    c = my_malloc((size_t)10);
    my_free(a);
    my_set_policy(NEXT_FIT);
    d = my_malloc((size_t)10);
    e = my_malloc((size_t)65300);
    f = my_malloc((size_t)100);
    g = my_malloc((size_t)100);
    my_heap_dump();
    return 0;
}
