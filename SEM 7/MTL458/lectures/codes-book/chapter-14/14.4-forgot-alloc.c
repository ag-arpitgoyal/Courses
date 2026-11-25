#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char *src = "hello";

    /* BAD — book section 14.4: forgetting to allocate (do NOT run this block)
    char *dst;
    strcpy(dst, src);
    */

    /* GOOD — allocate before copy */
    char *dst = (char *)malloc(strlen(src) + 1);
    if (dst == NULL) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    strcpy(dst, src);
    printf("dst = %s at %p\n", dst, (void *)dst);
    free(dst);
    return 0;
}

/*
DEMO — Section 14.4: Forgetting To Allocate Memory

Compile:
  gcc -o forgot-alloc 14.4-forgot-alloc.c -Wall

Run:
  ./forgot-alloc

Example output:
  dst = hello at 0x....

Bad version (commented out in source):
  char *dst;           // uninitialized pointer
  strcpy(dst, src);    // segfault — writes to random address

Good version:
  dst = malloc(strlen(src) + 1);
  strcpy(dst, src);

Book tip:
  strdup(src) allocates and copies in one step.

Key lesson:
  It compiled or it ran != it is correct (book tip in section 14.4).
*/
