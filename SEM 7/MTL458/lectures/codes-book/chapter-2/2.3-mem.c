#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

/* from book's common.h — busy-wait for ~howlong seconds */
double GetTime(void)
{
    struct timeval t;
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);
    return t.tv_sec + t.tv_usec / 1000000.0;
}

void Spin(int howlong)
{
    double t = GetTime();
    while ((GetTime() - t) < (double)howlong)
        ;
}

int main(int argc, char *argv[])
{
    int *p = malloc(sizeof(int));    /* a1: allocate memory on the heap */
    assert(p != NULL);
    /* a2: print PID and virtual address.
     * On modern Linux, ASLR is on by default (randomize_va_space=2), so two
     * ./mem processes often get DIFFERENT heap addresses — that is normal.
     * Each process still has its own private *p; counters stay independent. */
    printf("(%d) address of p: %08x\n",
           getpid(), (unsigned)p);
    *p = 0;                          /* a3: store zero at that address */
    while (1) {
        Spin(1);
        *p = *p + 1;
        printf("(%d) p: %d\n", getpid(), *p);  /* a4: increment and print */
    }
    return 0;
}

/*
DEMO — Figure 2.3 & 2.4: Virtualizing Memory

Compile:
  gcc -o mem 2.3-mem.c -Wall

Run one process:
  ./mem

  Example output:
    (2134) address of p: 00200000
    (2134) p: 1
    (2134) p: 2
    ...

  Each line waits ~1 second. The number in parentheses is the process ID (PID).
  Stop with Ctrl-C.

Run two processes at once:
  ./mem & ; ./mem &

  bash equivalent:
  ./mem & ./mem &

  Example output:
    (24113) address of p: 00200000
    (24114) address of p: 00200000
    (24113) p: 1
    (24114) p: 1
    (24113) p: 2
    (24114) p: 2
    ...

What happens:
  Each process has its own private copy of *p. Incrementing *p in one
  process does not affect the other — that is memory virtualization.

  The book shows both processes printing the SAME address (e.g. 00200000).
  On modern Linux you will usually see DIFFERENT addresses because ASLR
  randomizes heap/stack placement per process. That is expected; the demo
  still works (both counters go 1, 2, 3... independently).

Why:
  The OS gives each process its own virtual address space and maps it
  onto physical RAM separately (memory virtualization).

ASLR note (same address as in the book):
  Check:  cat /proc/sys/kernel/randomize_va_space   (2 = full ASLR, default)
  To match the book's identical-address output, temporarily disable ASLR:
    sudo sysctl -w kernel.randomize_va_space=0
  Re-enable afterward:
    sudo sysctl -w kernel.randomize_va_space=2
*/
