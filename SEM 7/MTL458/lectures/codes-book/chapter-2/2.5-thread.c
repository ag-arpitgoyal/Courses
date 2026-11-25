#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

/* from book's common_threads.h — pthread wrappers that check return codes */
#define Pthread_create(thread, attr, start_routine, arg) \
    assert(pthread_create(thread, attr, start_routine, arg) == 0)
#define Pthread_join(thread, value_ptr) \
    assert(pthread_join(thread, value_ptr) == 0)

volatile int counter = 0;
int loops;

void *worker(void *arg)
{
    int i;
    for (i = 0; i < loops; i++) {
        counter++;
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: threads <value>\n");
        exit(1);
    }
    loops = atoi(argv[1]);
    pthread_t p1, p2;
    printf("Initial value : %d\n", counter);

    Pthread_create(&p1, NULL, worker, NULL);
    Pthread_create(&p2, NULL, worker, NULL);
    Pthread_join(p1, NULL);
    Pthread_join(p2, NULL);
    printf("Final value   : %d\n", counter);
    return 0;
}

/*
DEMO — Figure 2.5: Concurrency (race condition)

Compile:
  gcc -o thread 2.5-thread.c -Wall -pthread

Run with a small loop count (works correctly):
  ./thread 1000

  Example output:
    Initial value : 0
    Final value   : 2000

  Each of two threads increments counter 1000 times → 2000 total. Good.

Run with a large loop count (broken / non-deterministic):
  ./thread 100000

  Example output:
    Initial value : 0
    Final value   : 143012

  Run again — you may get a different wrong number, or occasionally 200000.

What happens:
  counter++ is NOT atomic. It takes roughly three steps:
    1. load counter from memory into a register
    2. add 1 in the register
    3. store the result back to memory

  The OS can context-switch between threads between any of these steps,
  so updates are lost and the final count is wrong.

  Example interleaving (both threads do counter++; start counter = 50):

    Step | Thread A        | Thread B        | counter in memory
    -----|-----------------|-----------------|------------------
       1 | Load 50         |                 | 50
       2 |                 | Load 50         | 50
       3 | Add → 51        |                 | 50
       4 |                 | Add → 51        | 50
       5 | Store 51        |                 | 51
       6 |                 | Store 51        | 51

  Both threads read 50 and both write 51. Two increments in code, but
  memory only went 50 → 51 — one update was lost.

Why:
  This is the core concurrency problem: shared memory + interleaved
  execution without synchronization. Later chapters cover locks and other fixes.
*/
