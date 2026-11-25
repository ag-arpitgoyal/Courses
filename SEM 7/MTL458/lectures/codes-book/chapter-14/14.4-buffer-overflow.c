#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char *src = "hello";

    /* BAD — book: not enough room (no space for '\\0') */
    char *dst = (char *)malloc(strlen(src));
    if (dst == NULL)
        return 1;

    strcpy(dst, src); /* may "work" sometimes — buffer overflow */
    printf("dst = %s\n", dst);

    free(dst);
    return 0;
}

/*
DEMO — Section 14.4: Not Allocating Enough Memory (buffer overflow)

Compile:
  gcc -o buffer-overflow 14.4-buffer-overflow.c -Wall

Run:
  ./buffer-overflow

  May print "hello" correctly — that does NOT mean it is safe.
  One byte past the allocation is overwritten (missing null terminator room).

Correct allocation from book:
  malloc(strlen(src) + 1)

Why dangerous:
  Heap overflows can corrupt allocator metadata → crashes or security bugs.

Debug with:
  valgrind ./buffer-overflow
*/
