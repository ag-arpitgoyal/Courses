/* leak_repeat: leak report has no side effects and can be called repeatedly; "1 blocks" format */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    char *a, *b;
    a = my_malloc(10);
    b = my_malloc(20);
    my_leak_report();
    my_leak_report();
    my_free(a);
    my_leak_report();
    my_free(b);
    my_leak_report();
    (void)a; (void)b;
    return 0;
}
