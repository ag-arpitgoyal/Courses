#include "myalloc.h"
#include <stdio.h>

typedef struct Node {
    int val;
    struct Node *next;
} Node;

int main(void) {
    my_set_policy(BEST_FIT);
    Node *head = NULL;
    Node *tail = NULL;

    for (int i = 1; i <= 100; i++) {
        Node *n = my_malloc(sizeof(Node));
        n->val = i;
        n->next = NULL;
        if (!head) {
            head = tail = n;
        } else {
            tail->next = n;
            tail = n;
        }
    }

    /* Remove all even-valued nodes and free them */
    Node *curr = head;
    Node *prev = NULL;
    while (curr) {
        Node *next = curr->next;
        if (curr->val % 2 == 0) {
            if (prev) prev->next = next;
            else head = next;
            my_free(curr);
        } else {
            prev = curr;
        }
        curr = next;
    }

    /* Allocate 25 new nodes with values 1000..1024 and prepend them */
    for (int i = 0; i < 25; i++) {
        Node *n = my_malloc(sizeof(Node));
        n->val = 1000 + i;
        n->next = head;
        head = n;
    }

    /* Sum all remaining nodes:
       Odd numbers in 1..100: 50 odds summing to 50^2 = 2500.
       Plus 25 nodes 1000..1024: 25 * 1000 + (24 * 25 / 2) = 25000 + 300 = 25300.
       Total = 27800, count = 75. */
    int count = 0;
    int sum = 0;
    curr = head;
    while (curr) {
        Node *next = curr->next;
        count++;
        sum += curr->val;
        my_free(curr);
        curr = next;
    }

    printf("count=%d sum=%d\n", count, sum);
    my_leak_report();
    return 0;
}
