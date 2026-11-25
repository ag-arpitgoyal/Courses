#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    int fd = open("/tmp/file", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    assert(fd > -1);
    int rc = write(fd, "hello world\n", 13);
    assert(rc == 13);
    close(fd);
    return 0;
}

/*
DEMO — Figure 2.6: Persistence (file I/O via system calls)

Compile:
  gcc -o io 2.6-io.c -Wall

Run:
  ./io

  No output on success. The program exits quietly.

Verify the file was created:
  cat /tmp/file

  Expected output:
    hello world

What happens step by step:
  1. open("/tmp/file", ...)
       Asks the OS to create (or truncate) a file and return a file descriptor.
  2. write(fd, "hello world\n", 13)
       Asks the OS file system to store 13 bytes of data.
  3. close(fd)
       Tells the OS this program is done with that file.

Why:
  Data in RAM is volatile (lost on crash/power-off). The OS file system
  persists data on disk so it survives after the program exits.

Related workflow from the book (files shared across programs):
  emacs -nw main.c          # editor creates a source file
  gcc -o main main.c        # compiler reads it, writes an executable
  ./main                    # shell runs the new program
*/
