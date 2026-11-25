/* guard_realloc_same_size: my_realloc(p, same n) checks guard, returns p and resets the guard */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *a2;
    a = my_malloc(10);
    ((unsigned char *)a)[14] = 0x00;
    a2 = my_realloc(a, 10);
    printf("a2==a %d\n", a2 == a);
    my_free(a2);
    my_heap_dump();
    (void)a; (void)a2;
    return 0;
}
