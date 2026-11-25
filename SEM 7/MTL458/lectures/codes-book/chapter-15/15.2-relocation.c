/*
 * Section 15.2 — example from Figure 15.1 / 15.2
 * C source for: x = x + 3;
 *
 * Assumes register ebx holds the address of x (virtual address 15 KB
 * in the book's 16 KB address space example).
 */

void func(void)
{
    int x;
    /* ... */
    x = x + 3; /* line of interest */
}

/*
 * Disassembly the book gives (x86), when ebx -> address of x:
 *
 *   128: movl 0x0(%ebx), %eax    ; load x into eax
 *   132: addl $0x03, %eax        ; add 3
 *   135: movl %eax, 0x0(%ebx)    ; store back to x
 *
 * Virtual addresses referenced:
 *   PC at 128, 132, 135 (code section)
 *   Data at 15 KB (stack) for load/store of x
 *
 * With base register = 32 KB (32768), hardware translates:
 *   virtual 128   -> physical 32896  (fetch instruction)
 *   virtual 15 KB -> physical 47 KB  (load/store x)
 *
 * Formula: physical_address = virtual_address + base
 * (bounds register checks virtual < segment size)
 */

/*
DEMO — Section 15.2: Address Translation Example (Figures 15.1 & 15.2)

This file is reference material — the book shows C + assembly, not a full program.

Compile (syntax check only):
  gcc -c 15.2-relocation.c -Wall

Disassemble your own program to see real loads/stores:
  gcc -O0 -g -o demo 15.2-relocation.c -Wall
  objdump -d demo | less

On macOS:
  otool -tv demo

What to notice:
  Each memory access uses virtual addresses from the process's view.
  With dynamic relocation (base + bounds), hardware adds base on every access.

Initial value of x in book figure: 3000 → after x = x + 3 → 3003.

Next chapter (16) generalizes base/bounds to per-segment registers.
*/
