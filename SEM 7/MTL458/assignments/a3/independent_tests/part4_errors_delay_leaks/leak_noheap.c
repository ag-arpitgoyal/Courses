/* leak_noheap: leak report before any region exists prints just the zero summary */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"


int main(void)
{
    my_leak_report();
    my_heap_dump();
    printf("end\n");
    return 0;
}
