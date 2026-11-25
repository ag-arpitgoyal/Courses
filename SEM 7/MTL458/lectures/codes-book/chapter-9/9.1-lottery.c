#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* global ticket count */
int gtickets = 0;

typedef struct node_t {
    int tickets;
    struct node_t *next;
} node_t;

node_t *head = NULL;

void insert(int tickets)
{
    node_t *tmp = malloc(sizeof(node_t));
    assert(tmp != NULL);
    tmp->tickets = tickets;
    tmp->next = head;
    head = tmp;
    gtickets += tickets;
}

void print_list(void)
{
    node_t *curr = head;
    printf("List: ");
    while (curr) {
        printf("[%d] ", curr->tickets);
        curr = curr->next;
    }
    printf("\n");
}

/* Figure 9.1 core: pick lottery winner from ticket list */
node_t *pick_winner(int winner)
{
    int counter = 0;
    node_t *current = head;

    while (current) {
        counter = counter + current->tickets;
        if (counter > winner)
            break;
        current = current->next;
    }
    return current;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: lottery <seed> <loops>\n");
        exit(1);
    }
    int seed = atoi(argv[1]);
    int loops = atoi(argv[2]);
    srandom(seed);

    /* A=100, B=50, C=250 tickets (book example; order in list matters) */
    insert(50);
    insert(100);
    insert(250);

    print_list();
    printf("total tickets: %d\n\n", gtickets);

    for (int i = 0; i < loops; i++) {
        int winner = random() % gtickets;
        node_t *current = pick_winner(winner);
        printf("draw %d -> winner tickets [%d]\n", winner, current->tickets);
    }
    return 0;
}

/*
DEMO — Figure 9.1: Lottery Scheduling Decision Code

Compile:
  gcc -o lottery 9.1-lottery.c -Wall

Run (seed 1, one lottery draw):
  ./lottery 1 1

Run (seed 1, ten draws):
  ./lottery 1 10

Example setup from book:
  Job A: 100 tickets, Job B: 50, Job C: 250 → total 400 tickets.

How winner selection works (Figure 9.1):
  1. Pick random winner in [0, totaltickets).
  2. Walk the linked list, adding each job's tickets to counter.
  3. First job where counter > winner is the winner.

Book walk-through (winner = 300):
  counter = 100 after A  → 300 > 100, continue
  counter = 150 after B  → 300 > 150, continue
  counter = 400 after C  → 400 > 300, stop → C wins

Key idea:
  Jobs receive CPU time in proportion to their tickets over many runs.
  Simple, proportional-share scheduling policy.
*/
