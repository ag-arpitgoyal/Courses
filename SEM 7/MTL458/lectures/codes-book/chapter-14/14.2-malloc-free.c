#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *arr = (int *)malloc(10 * sizeof(int));
    if (arr == NULL) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    for (int i = 0; i < 10; i++)
        arr[i] = i * i;

    printf("arr[5] = %d\n", arr[5]);

    free(arr);
    return 0;
}

/*
DEMO — Sections 14.2 & 14.3: malloc() and free()

Compile:
  gcc -o malloc-demo 14.2-malloc-free.c -Wall

Run:
  ./malloc-demo

Example output:
  arr[5] = 25

What happens:
  1. malloc(10 * sizeof(int)) asks the C library for heap space.
  2. Library may call brk/mmap internally; not a direct syscall every time.
  3. free(arr) returns the chunk to the allocator (not always to OS immediately).

API reminder from book:
  void *malloc(size_t size);
  void free(void *ptr);

Tip:
  Always check for NULL after malloc on critical paths.
*/
