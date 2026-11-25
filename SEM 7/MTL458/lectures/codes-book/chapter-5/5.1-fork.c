#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("hello world (pid:%d)\n", (int)getpid());
    int rc = fork();
    if (rc < 0) {
        /* fork failed; exit */
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        /* child (new process) */
        printf("hello, I am child (pid:%d)\n", (int)getpid());
    } else {
        /* parent goes down this path (main) */
        printf("hello, I am parent of %d (pid:%d)\n",
               rc, (int)getpid());
    }
    return 0;
}

/*
DEMO — Figure 5.1: p1.c — Calling fork()

Compile:
  gcc -o fork-demo 5.1-fork.c -Wall

Run:
  ./fork-demo

Example output (PIDs will differ on your machine):
  hello world (pid:29146)
  hello, I am parent of 29147 (pid:29146)
  hello, I am child (pid:29147)

  OR (order can flip — see below):
  hello world (pid:29146)
  hello, I am child (pid:29147)
  hello, I am parent of 29147 (pid:29146)

What happens:
  1. Process prints its PID and calls fork().
  2. fork() creates a nearly exact copy of the process (the child).
  3. Both parent and child continue from the line AFTER fork().
  4. fork() returns twice:
       - parent gets the child's PID (rc > 0)
       - child gets 0 (rc == 0)

Why output order is non-deterministic:
  After fork(), two processes compete for the CPU. The scheduler may run
  the child or parent first, so print order is not guaranteed without wait().

Key idea:
  fork() is how UNIX creates a new process — the child starts at the same
  point in the code, not at main() again (hence only one "hello world").
*/
