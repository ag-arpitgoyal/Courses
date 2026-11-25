#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *p = (int *)malloc(sizeof(int));
    if (p == NULL)
        return 1;

    /* forgot to *p = 0; before use — uninitialized read */
    printf("uninitialized value might be: %d\n", *p);

    *p = 42;
    printf("after init: %d\n", *p);

    free(p);
    return 0;
}

/*
DEMO — Section 14.4: Forgetting to Initialize Allocated Memory

Compile:
  gcc -o uninit 14.4-uninitialized.c -Wall

Run:
  ./uninit

Example output (varies every run):
  uninitialized value might be: 0
  after init: 42

  OR garbage like 1839201 on another run.

Key idea:
  malloc() does NOT zero memory (use calloc() if you need zeroed memory).

Fix:
  *p = 0; immediately after malloc, or use calloc(1, sizeof(int)).
*/
