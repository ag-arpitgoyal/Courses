#include <stdio.h>
#include <stdlib.h>

typedef struct __Node_t {
    int value;
    struct __Node_t *next;
} Node_t;

Node_t *head = NULL;

void List_Push(int value)
{
    Node_t *n = malloc(sizeof(Node_t));
    n->value = value;
    n->next = head;
    head = n;
}

int List_Pop(void)
{
    Node_t *tmp = head;
    int value = head->value;
    head = head->next;
    free(tmp);
    return value;
}

int main(void)
{
    List_Push(30);
    List_Push(20);
    List_Push(10);

    printf("pop: %d\n", List_Pop());
    printf("pop: %d\n", List_Pop());
    printf("pop: %d\n", List_Pop());
    return 0;
}

/*
DEMO — Figure 10.3: Simple List Delete Code

Compile:
  gcc -o list-pop 10.3-list-pop.c -Wall

Run:
  ./list-pop

Example output:
  pop: 10
  pop: 20
  pop: 30

What List_Pop does (book Figure 10.3):
  1. Remember old head pointer (tmp).
  2. Read value at head.
  3. Advance head to head->next.
  4. free(old head).
  5. Return the value.

Why the book shows this on multiprocessors:
  If two CPUs call List_Pop() on the same shared list WITHOUT locking,
  both may read the same head, double-free memory, or lose nodes.

Fix mentioned in book:
  Wrap with pthread_mutex_lock / unlock around the whole routine.

Try with threads (optional, shows races):
  gcc -o list-pop-race 10.3-list-pop.c -Wall -pthread
  Run many threads calling List_Pop — may crash or give wrong results
  unless you add a mutex.
*/
