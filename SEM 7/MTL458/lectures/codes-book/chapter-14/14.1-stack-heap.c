#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void stack_example(void)
{
    int x; /* stack — freed when func returns */
    x = 42;
    printf("stack int x = %d at %p\n", x, (void *)&x);
}

void heap_example(void)
{
    int *x = (int *)malloc(sizeof(int)); /* heap — must free explicitly */
    *x = 42;
    printf("heap int *x = %d at %p\n", *x, (void *)x);
    free(x);
}

int main(void)
{
    stack_example();
    heap_example();
    return 0;
}

/*
DEMO — Section 14.1: Stack vs Heap Memory

Compile:
  gcc -o stack-heap 14.1-stack-heap.c -Wall

Run:
  ./stack-heap

Stack (automatic):
  int x; in a function — compiler allocates/deallocates on function entry/exit.
  Does not survive after the function returns.

Heap (explicit):
  malloc() / free() — you manage lifetime.
  Survives until you free it (or process exits).

Key idea from chapter:
  Use stack for short-lived locals; use heap when data must outlive the call.
*/
