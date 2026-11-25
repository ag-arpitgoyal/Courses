#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

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
        int wc = wait(NULL);
        printf("hello, I am parent of %d (wc:%d) (pid:%d)\n",
               rc, wc, (int)getpid());
    }
    return 0;
}

/*
DEMO — Figure 5.2: p2.c — Calling fork() and wait()

Compile:
  gcc -o wait-demo 5.2-wait.c -Wall

Run:
  ./wait-demo

Example output (deterministic order now):
  hello world (pid:29266)
  hello, I am child (pid:29267)
  hello, I am parent of 29267 (wc:29267) (pid:29266)

What happens:
  1. Same fork() setup as 5.1-fork.c.
  2. Parent calls wait(NULL) before printing its second message.
  3. wait() blocks until the child exits.
  4. wait() returns the child's PID (stored in wc).

Why child always prints before parent now:
  Even if the parent runs first, it blocks in wait() until the child finishes.
  So the child's printf always completes before the parent's second printf.

Key idea:
  wait() lets a parent synchronize with — and reap — a finished child process.
*/
