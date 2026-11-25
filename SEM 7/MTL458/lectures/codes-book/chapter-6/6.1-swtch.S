/*
 * Figure 6.1 — xv6 context switch code (x86 assembly)
 * void swtch(struct context **old, struct context *new);
 *
 * Saves current register context in *old, loads registers from new,
 * then returns into the new context.
 */

.globl swtch
swtch:
    /* Save old registers */
    movl 4(%esp), %eax       /* put old ptr into eax */
    popl 0(%eax)             /* save the old IP */
    movl %esp, 4(%eax)       /* and stack */
    movl %ebx, 8(%eax)       /* and other registers */
    movl %ecx, 12(%eax)
    movl %edx, 16(%eax)
    movl %esi, 20(%eax)
    movl %edi, 24(%eax)
    movl %ebp, 28(%eax)

    /* Load new registers */
    movl 4(%esp), %eax       /* put new ptr into eax */
    movl 28(%eax), %ebp      /* restore other registers */
    movl 24(%eax), %edi
    movl 20(%eax), %esi
    movl 16(%eax), %edx
    movl 12(%eax), %ecx
    movl 8(%eax), %ebx
    movl 4(%eax), %esp       /* stack is switched here */
    pushl 0(%eax)            /* return addr put in place */
    ret                      /* finally return into new ctxt */

/*
DEMO — Figure 6.1: The xv6 Context Switch Code

This is kernel assembly from xv6, not a standalone user program.

What it does:
  1. Save the current CPU registers into the old process's struct context.
  2. Load registers from the new process's struct context.
  3. Switch the kernel stack pointer and return — now running process B.

When it runs:
  After a timer interrupt, the OS decides to switch processes and calls
  swtch(&procA->context, procB->context). When swtch returns, the CPU
  is executing in B's kernel trap path, not A's.

Why two saves/restores:
  - Hardware saves user registers on trap into the kernel (onto kernel stack).
  - swtch saves/restores kernel registers between process struct entries.

Assemble only (needs x86 32-bit toolchain; may not link without xv6):
  as -32 -o 6.1-swtch.o 6.1-swtch.S

On 64-bit Linux you may need: gcc -m32 -c 6.1-swtch.S

Learn more:
  xv6: https://pdos.csail.mit.edu/6.828/
  Pair with chapter-4/4.3-xv6-proc.c (struct context / struct proc).
*/
