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
    if (argc != 2) {
        fprintf(stderr, "usage: cpu <string>\n");
        exit(1);
    }
    char *str = argv[1];
    while (1) {
        Spin(1);
        printf("%s\n", str);
    }
    return 0;
}

/*
DEMO — Figure 2.1 & 2.2: Virtualizing the CPU

Compile:
  gcc -o cpu 2.1-cpu.c -Wall

Run one process (prints "A" once per second forever):
  ./cpu "A"

  The program spins for one second, prints the string, repeats.
  Stop it with Ctrl-C.

Run four processes at once (tcsh-style background jobs):
  ./cpu A & ; ./cpu B & ; ./cpu C & ; ./cpu D &

  bash equivalent:
  ./cpu A & ./cpu B & ./cpu C & ./cpu D &

What happens:
  Even with ONE physical CPU, all four programs appear to run together.
  Their output (A, B, C, D) interleaves on the terminal.

Why:
  The OS rapidly switches between processes (CPU virtualization).
  Each process thinks it has its own CPU; in reality they time-share one.
*/
