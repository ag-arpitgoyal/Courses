/* inv_region1: invalid/double-free pointers and guard overflow in region 1 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *big, *g, *h;
    a = my_malloc(10);
    big = my_malloc(70000);
    my_free(big + 1);
    my_free(big - 32);
    my_free(big + 70048);
    my_free(big);
    my_free(big);
    g = my_malloc(70000);
    h = my_malloc(100);
    printf("g==big %d\n", g == big);
    my_heap_dump();
    ((unsigned char *)g)[70003] = 0x00;
    my_free(g);
    my_free(g);
    my_heap_dump();
    (void)a; (void)big; (void)g; (void)h;
    return 0;
}
