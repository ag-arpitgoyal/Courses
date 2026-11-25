#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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

        /* Step 2: build argument list (like running "wc 5.3-exec.c" on the shell) */
        char *myargs[3];
        myargs[0] = strdup("wc");           /* argv[0]: program name */
        myargs[1] = strdup("5.3-exec.c"); /* argv[1]: file for wc to count */
        myargs[2] = NULL;                   /* argv must end with NULL */

        /* Step 3: execvp replaces this process with wc (same PID, new program).
         * "p" = search PATH for wc; on success, code below never runs. */
        execvp(myargs[0], myargs);
        printf("this shouldn't print out");  /* only if execvp fails */
    } else {
        /* parent goes down this path (main) */
        int wc = wait(NULL);
        printf("hello, I am parent of %d (wc:%d) (pid:%d)\n",
               rc, wc, (int)getpid());
    }
    return 0;
}

/*
DEMO — Figure 5.3: p3.c — fork(), wait(), and exec()

Compile:
  gcc -o exec-demo 5.3-exec.c -Wall

Run (from this directory so wc can find the source file):
  ./exec-demo

Example output:
  hello world (pid:29383)
  hello, I am child (pid:29384)
  29 107 1030 5.3-exec.c
  hello, I am parent of 29384 (wc:29384) (pid:29383)

  (line counts vary; book showed counts for p3.c)

What happens:
  1. fork() creates a child.
  2. Child calls execvp("wc", ...) — replaces its own program with wc.
  3. wc reads 5.3-exec.c and prints lines, words, bytes.
  4. "this shouldn't print out" never runs if exec succeeds.
  5. Parent wait()s, then prints.

Key idea:
  fork() creates a new process; exec() loads a different program into it.
  Together they let a shell run commands: fork a child, then exec the program.

Note:
  execvp() searches PATH for wc. Requires wc installed (standard on Linux/macOS).
*/
