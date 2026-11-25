# Free-Space Management

## Introduction

A memory allocator library provides `my_malloc` and `my_free`, which behave like `malloc` and `free` without ever calling them. Your library obtains memory from the operating system with `mmap` and manages it itself.

## Environment and Build Requirements

- Every function declared in the provided `myalloc.h` is compiled with GCC 11.1 or later under the C17 standard on Linux, with no third-party libraries. Print all output to standard output using `printf`.
- Obtain all memory with `mmap`, including memory for your own data structures. Do not use `brk`, `sbrk`, `malloc`, `free` or any other library function that allocates memory.

## The Heap

The heap is made of **regions**, each obtained with one `mmap` call. The first region is created by the first request, with size 65536 bytes. Regions are numbered 0, 1, 2, ... in the order they are created, and are never unmapped. The first 32 bytes of a region are yours to use as you like. The rest is divided into **blocks**, laid end to end with no gaps, so a new region holds a single free block at offset 32.

Every block starts with a 32-byte header, whose contents are up to you but must include a magic number. The pointer returned to the user is the address just after the header. A request for `n` bytes is held in a block of size

```text
block_size(n) = max(128, round_up(32 + n + 8, 16))
```

which is the header, the `n` bytes, 8 guard bytes (Part 4), and padding to a multiple of 16.

A block is **allocated**, **free**, or, from Part 4, **delayed**. Its **offset** is the position of its header from the start of its region, and it is written `region:offset` in all output, for example `0:160`. **Heap order** sorts blocks by region number, and by offset within a region: `0:32`, `0:160`, ..., `1:32`, `1:160`, ... "First", "next" and "earliest" always refer to this order, not to memory addresses.

When `my_malloc(n)` is called, the allocator needs a block of size `B = block_size(n)`. It picks a free block of size `F >= B`, chosen by the current policy (Part 2). Your allocator follows these three rules when allocating and freeing blocks:

- **Splitting.** If `F - B >= 128`, the first `B` bytes become the allocated block and the remaining `F - B` bytes stay free. Otherwise all `F` bytes are allocated.
- **Growth.** If no free block is large enough, create a new region whose size is the smallest multiple of 65536 that is at least `B + 32`, and place the request in it.
- **Coalescing.** Whenever a block becomes free, merge it with the free blocks immediately before and after it, if there are any. Blocks in different regions are never merged.

`my_malloc` returns `NULL` if `n` is 0 or greater than 2^30. `my_free(NULL)` does nothing.

**Heap dump.** `my_heap_dump()` prints each region as `region <number> <size>`, followed by each of its blocks in heap order, each on its own line starting with two spaces, as `<offset> A <size> <requested>` if allocated, `<offset> F <size>` if free, and `<offset> D <size> <requested>` if delayed. `<size>` includes the header. If no region exists yet, it prints nothing. Tests check your heap dumps, so match this format exactly. Some tests are also ordinary programs that use your functions in place of the standard `malloc`, `calloc`, `realloc` and `free`, and check only their own output.

## Part 1: The Core Allocator (15 points)

Every request uses the default policy, first fit: place each request in the first free block that is large enough.

**Input:**

```c
a = my_malloc(10);
b = my_malloc(100);
c = my_malloc(10);
my_heap_dump();
my_free(b);
my_free(a);
d = my_malloc(100000);
my_heap_dump();
```

**Expected Output:**

```text
region 0 65536
  32 A 128 10
  160 A 144 100
  304 A 128 10
  432 F 65104
region 0 65536
  32 F 272
  304 A 128 10
  432 F 65104
region 1 131072
  32 A 100048 100000
  100080 F 30992
```

## Part 2: Placement Policies (15 points)

When several free blocks are large enough, the **placement policy** decides which one a request uses. `my_set_policy` sets the policy for all later requests, and it may be changed between any two calls. Among the free blocks of size at least `B`:

- **First fit** (`FIRST_FIT`) chooses the first.
- **Next fit** (`NEXT_FIT`) chooses the first one that ends after the next-fit position, wrapping around to the start of the heap if there is none.
- **Best fit** (`BEST_FIT`) chooses the smallest, and the earliest of those if several are equally small.
- **Worst fit** (`WORST_FIT`) chooses the largest free block, and the earliest of those if several are equally large. If it is smaller than `B`, the heap grows.

**The next-fit position** starts at `0:0`. After every new allocation, under any policy, it moves to the end of the new block.

The following example runs under the default policy, first fit.

**Input:**

```c
a  = my_malloc(248);
x1 = my_malloc(10);
b  = my_malloc(120);
x2 = my_malloc(10);
c  = my_malloc(360);
x3 = my_malloc(10);
my_free(b);
y  = my_malloc(120);
my_free(y);
my_free(a);
my_free(c);
my_heap_dump();
```

**Expected Output:**

```text
region 0 65536
  32 F 288
  320 A 128 10
  448 F 160
  608 A 128 10
  736 F 400
  1136 A 128 10
  1264 F 64272
```

Under first fit, `y` was placed where `b` had been, so the next-fit position is now at `0:608`. If the policy is now changed, a request for 100 bytes (block size 144) is placed differently by each policy:

| Policy | Chosen block | Result |
| --- | --- | --- |
| First fit | `0:32`, size 288 | `32 A 144 100` and `176 F 144` |
| Next fit | `0:736`, size 400 | `736 A 144 100` and `880 F 256` |
| Best fit | `0:448`, size 160 | `448 A 160 100`, not split since only 16 bytes would be left |
| Worst fit | `0:1264`, size 64272 | `1264 A 144 100` and `1408 F 64128` |

## Part 3: calloc and realloc (10 points)

`calloc` allocates an array whose bytes all start at zero, for example a table of counters. `realloc` changes the size of an existing allocation while keeping its data, for example to grow a buffer as more input arrives.

**`my_calloc(nmemb, size)`** allocates `nmemb * size` bytes as `my_malloc` would and sets them to zero. It returns `NULL` if the product is 0, greater than 2^30, or overflows `size_t`.

**`my_realloc(ptr, n)`** first handles these cases:

- If `ptr` is `NULL`, it behaves as `my_malloc(n)`.
- If `ptr` is not the start of an allocated block's data, it reports the error as in Part 4 and returns `NULL`.
- If `n` is 0, it frees the block as `my_free` would and returns `NULL`.
- If `n` is greater than 2^30, it returns `NULL` and leaves the block unchanged.

Otherwise, let `S` be the current size of the block and `B = block_size(n)` the size it needs for `n` bytes, and use the first of these that applies:

1. **Shrink in place** if `B <= S`. If `S - B >= 128`, split off the last `S - B` bytes as a free block and coalesce it with the next block if that is free. Return `ptr`.
2. **Grow in place** if the next block in the same region is free and the two together are at least `B`. Merge them, split off any leftover by the Splitting rule, and return `ptr`. The block before `ptr` is never used.
3. **Move** otherwise. Place a new block for `n` bytes using the current policy, copy the data into it, free the old block as `my_free` would, and return the new pointer. The old block is never reused for the new one.

**Input:**

```c
p = my_malloc(100);
q = my_malloc(200);
r = my_malloc(10);
my_free(q);
p = my_realloc(p, 250);
my_heap_dump();
p = my_realloc(p, 20);
my_heap_dump();
```

**Expected Output:**

```text
region 0 65536
  32 A 384 250
  416 A 128 10
  544 F 64992
region 0 65536
  32 A 128 20
  160 F 256
  416 A 128 10
  544 F 64992
```

## Part 4: Detecting Errors (20 points)

Your allocator detects some common mistakes programs make with `malloc` and `free`, using the techniques described below. Each message is printed on its own line. Tests make these mistakes only in the ways described: they write past a block only into its guard bytes, and never overwrite or fake a header.

### Invalid pointers and double frees

When `my_free` or `my_realloc` is given a pointer other than `NULL`, it is valid if it is the start of an allocated block's data. If it is the start of a free or delayed block's data, print `error: double free (block <region>:<offset>)`. Otherwise, including pointers outside your heap, into the middle of a block, or to the stack, print `error: invalid pointer`. After either error the call does nothing more.

For example, if blocks `a` and `b` are next to each other and both are freed, they merge into one free block starting at `a`. Freeing `a` again is then a `double free`, but freeing `b` again is an `invalid pointer`, because `b` is no longer the start of any block.

### Overflows

A program overflows a block when it writes past the end of its `n` bytes. To detect this, the 8 bytes right after the user's `n` bytes are **guard bytes** (not the last 8 bytes of the block). Set them to `0xCA` whenever a block is allocated or resized, so after `my_realloc` they follow the new `n` bytes. When `my_free` or `my_realloc` is given a valid pointer, check the guard bytes first. If any of them has changed, print `error: overflow detected (block <region>:<offset>)`, then carry on with the call as normal. Print this error at most once per call.

### Dangling pointers

A **dangling pointer** is a pointer used after its block has been freed. To catch writes through one, freed blocks can be held in a **delay queue** before they really become free.

`my_set_delay(k)` sets the queue size, from 0 to 64. It is 0 by default, meaning blocks are freed immediately. While `k > 0`, freeing a valid pointer, by `my_free` or `my_realloc`, instead:

1. checks the guard bytes;
2. sets the block's `n` bytes to `0xDE`, marks it **delayed**, and adds it to the back of the queue;
3. if the queue now holds more than `k` blocks, removes the front one. If any of its `n` bytes is no longer `0xDE`, prints `error: write after free (block <region>:<offset>)`. Then frees and coalesces it as normal.

A delayed block is never reused or coalesced. Lowering `k` removes blocks from the front as in step 3 until the queue fits.

**Input:**

```c
my_set_delay(2);
a = my_malloc(10);
b = my_malloc(10);
c = my_malloc(10);
d = my_malloc(10);
my_free(a);
my_free(b);
b[3] = 'X';
my_free(c);
my_free(d);
my_heap_dump();
my_free(a);
my_free(b);
```

**Expected Output:**

```text
error: write after free (block 0:160)
region 0 65536
  32 F 256
  288 D 128 10
  416 D 128 10
  544 F 64992
error: double free (block 0:32)
error: invalid pointer
```

### Leak report

A **memory leak** happens when a program forgets to free memory it no longer needs. `my_leak_report()` is meant to be called when a program finishes, so every block still allocated at that point is a leak. It prints `leak: block <region>:<offset> (<n> bytes)` for each allocated block in heap order, then `leak summary: <blocks> blocks, <bytes> bytes`, where the bytes are the sum of the requested sizes. Free and delayed blocks are not leaks.

**Input:**

```c
a = my_malloc(100);
b = my_malloc(10);
c = my_malloc(10);
my_free(b);
my_leak_report();
```

**Expected Output:**

```text
leak: block 0:32 (100 bytes)
leak: block 0:304 (10 bytes)
leak summary: 2 blocks, 110 bytes
```

## Part 5: The Buddy Allocator (15 points)

A **buddy allocator** hands out only blocks whose sizes are powers of two. A free block is split into two equal halves, called **buddies**, and when both halves are free again they merge back. A test that uses it calls `my_set_policy(BUDDY)` first, then only `my_malloc`, `my_free` (with valid pointers) and `my_heap_dump`.

- **Memory.** The first request creates one region of 1048576 bytes with `mmap`. It never grows.
- **Block size.** Each block has a 16-byte header and no guard bytes. A request for `n` bytes uses a block of size `s`, the smallest power of two that is at least 32 and at least `n + 16`.
- **Allocating.** Take the smallest free block of size at least `s`, the one with the lowest offset if there are several. While it is larger than `s`, split it in half, keep the lower half and leave the upper half free.
- **Freeing.** The buddy of a block at offset `o` with size `s` is at offset `o XOR s`. If the buddy is free and the same size, merge the two, and repeat with the merged block.
- **Failure.** `my_malloc` returns `NULL` if `n` is 0, if `n + 16` is more than 1048576, or if no free block is large enough.
- **Dump.** `my_heap_dump()` prints `buddy 1048576`, then every block in order of offset in the usual format. Before the first request it prints nothing.

**Input:**

```c
my_set_policy(BUDDY);
a = my_malloc(40);
b = my_malloc(100);
c = my_malloc(40);
my_heap_dump();
```

**Expected Output:**

```text
buddy 1048576
  0 A 64 40
  64 A 64 40
  128 A 128 100
  256 F 256
  512 F 512
  1024 F 1024
  2048 F 2048
  4096 F 4096
  8192 F 8192
  16384 F 16384
  32768 F 32768
  65536 F 65536
  131072 F 131072
  262144 F 262144
  524288 F 524288
```

## Part 6: Performance (25 points)

With `N` the number of free blocks, your allocator must take amortized `O(log N)` time for `my_malloc` under best fit and worst fit, and for `my_free` under every policy. First fit and next fit may take linear time.

Each performance test builds a heap with at least 100,000 free blocks, then makes at least 100,000 more calls, and must finish within **10 seconds** with the correct final heap dump.

## Deliverable

Prepare one C file for this.
