/* ex_leak: task.md Part 4 leak-report example verbatim */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b, *c;
    a = my_malloc(100);
    b = my_malloc(10);
    c = my_malloc(10);
    my_free(b);
    my_leak_report();
    (void)a; (void)b; (void)c;
    return 0;
}
