#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int rc = fork();
    if (rc < 0) {
        /* fork failed; exit */
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        /* child: redirect standard output to a file */
        close(STDOUT_FILENO);
        open("./5.4-redirect.output", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);

        /* now exec "wc"... */
        char *myargs[3];
        myargs[0] = strdup("wc");               /* program: wc (word count) */
        myargs[1] = strdup("5.4-redirect.c");   /* argument: file to count */
        myargs[2] = NULL;                       /* marks end of array */
        execvp(myargs[0], myargs);              /* runs word count */
    } else {
        /* parent goes down this path (main) */
        int wc = wait(NULL);
        (void)wc;
    }
    return 0;
}

/*
DEMO — Figure 5.4: p4.c — fork(), wait(), exec(), and I/O redirection

Compile:
  gcc -o redirect-demo 5.4-redirect.c -Wall

Run:
  ./redirect-demo

  The shell prompt returns immediately with no screen output.

Verify output was redirected to a file:
  cat 5.4-redirect.output

Example file contents:
  32 109 846 5.4-redirect.c

  (numbers vary; book showed counts for p4.c)

What happens:
  1. fork() creates a child; parent wait()s.
  2. Child closes STDOUT (file descriptor 1).
  3. Child open()s 5.4-redirect.output — OS assigns it fd 1 (first free fd).
  4. Child execvp("wc", ...) — wc's printf goes to the file, not the terminal.
  5. Parent sees nothing on screen; results land in 5.4-redirect.output.

Why this works:
  UNIX assigns the lowest free file descriptor. After closing stdout,
  the next open() becomes the new stdout. This is how shells implement
  redirection, e.g.:  wc p4.c > newfile.txt

Key idea:
  Separating fork() and exec() lets the shell alter the child's environment
  (redirect I/O, set up pipes) between fork and exec.
*/  