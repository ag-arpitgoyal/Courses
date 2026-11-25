#include "myalloc.h"
#include <stdio.h>

int main(void) {
    /*
       Test double free vs invalid pointer:
       1. Before any region exists: my_free(stack_ptr) -> invalid pointer, no region created.
       2. Allocate a (0:32 A 128 10), b (0:160 A 128 10), c (0:288 A 128 10).
       3. Free a and b -> coalesce into 0:32 F 256.
          - my_free(a) -> double free (block 0:32)
          - my_free(b) -> invalid pointer (since b was coalesced into a!)
          - my_realloc(a, 20) -> double free (block 0:32), returns NULL
          - my_realloc(b, 20) -> invalid pointer, returns NULL
       4. Clarification 7: data address of a free block that was NEVER returned to the user
          (the leftover free block at 0:416 F 65120, whose data starts at (char*)c + 128):
          - my_free((char*)c + 128) -> double free (block 0:416)
          - my_realloc((char*)c + 128, 10) -> double free (block 0:416)
       5. Pointers to the header of c ((char*)c - 32), Region 0's first 32 bytes ((char*)a - 64),
          middle of c ((char*)c + 16), and one byte past region 0 ((char*)a - 64 + 65536):
          all print "error: invalid pointer".
    */
    int stack_var = 42;
    my_free(&stack_var);
    my_heap_dump();

    unsigned char *a = my_malloc(10);
    unsigned char *b = my_malloc(10);
    unsigned char *c = my_malloc(10);

    my_free(a);
    my_free(b);

    my_free(a);
    my_free(b);
    void *r1 = my_realloc(a, 20);
    void *r2 = my_realloc(b, 20);

    /* Leftover free block at offset 416 has its header at (c - 32) + 128 = c + 96,
       and its data address (header + 32) at c + 128! */
    my_free(c + 128);
    void *r3 = my_realloc(c + 128, 10);

    /* Invalid pointers: block header, region metadata, interior of allocated block, end of region */
    my_free(c - 32);
    my_free(a - 64);
    my_free(c + 16);
    my_free(a - 64 + 65536);
    printf("nulls=%d\n", (r1 == NULL && r2 == NULL && r3 == NULL));
    my_heap_dump();
    return 0;
}
