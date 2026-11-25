/* guard_delayed_no_recheck: overflowed block freed into delay queue: freeing it again is only a double free */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *q;
    my_set_delay(2);
    a = my_malloc(10);
    ((unsigned char *)a)[10] = 0x00;
    my_free(a);
    my_free(a);
    q = my_realloc(a, 5);
    printf("q %s\n", q ? "ptr" : "NULL");
    my_heap_dump();
    (void)a; (void)q;
    return 0;
}
