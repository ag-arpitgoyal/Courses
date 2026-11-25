#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    printf("location of code : %p\n", (void *)main);
    printf("location of heap : %p\n", (void *)malloc(1));
    int x = 3;
    printf("location of stack : %p\n", (void *)&x);
    return x;
}

/*
DEMO — Chapter 13 aside: Every Address You See Is Virtual

Compile:
  gcc -o va 13-aside-va.c -Wall

Run:
  ./va

Example output (64-bit Linux; addresses will differ):
  location of code : 0x5....
  location of heap : 0x5....
  location of stack : 0x7....

What you see:
  - code (main) lives in one region
  - heap (malloc) in another
  - stack (&x) at the high end of user space on typical Linux x86-64

Key idea:
  Every pointer a user program prints is a VIRTUAL address.
  The OS + hardware translate it to a physical address you never see.

Compare with chapter-2/2.3-mem.c (same virtual address across processes).
*/
