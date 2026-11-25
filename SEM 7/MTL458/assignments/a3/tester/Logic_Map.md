# Allocator logic map and assignment cross-check

Cross-check source: `Assignment 3 - Free-Space Management.pdf` (8 pages). The
instructor clarification supplied in this task resolves the initial-region
ambiguity: if the first allocation exceeds a 65,536-byte region, region 0 is
created at the growth-rule size.

## Control flow

```text
my_malloc(n)
  ├─ reject n == 0 or n > 2^30
  ├─ BUDDY policy -> allocate_buddy
  │    ├─ lazily mmap the fixed 1 MiB arena
  │    ├─ compute smallest power-of-two block >= max(32, n + 16)
  │    ├─ select lowest-offset block from smallest eligible order
  │    └─ split, retaining lower half; fail when arena cannot satisfy request
  └─ standard policy -> standard_block_size, allocate_standard
       ├─ choose_standard_block: first / next / best / worst fit
       ├─ when no fit, create_region(round_up(block_size + 32, 65536))
       ├─ split only if remainder >= 128
       ├─ mark allocated, write 8 0xCA guard bytes, advance next-fit cursor
       └─ return address immediately after 32-byte header

my_free(ptr)
  ├─ NULL -> no-op
  ├─ identify buddy or standard arena; validate exact user pointer/state
  ├─ invalid / already free or delayed -> required diagnostic, no mutation
  ├─ standard block -> check guard
  ├─ delay == 0 -> free and coalesce adjacent standard free blocks
  └─ delay > 0 -> fill user bytes 0xDE, enqueue as delayed; evict oldest
       ├─ check evicted bytes for write-after-free
       └─ free and coalesce evicted block

my_calloc -> validate multiplication and 0..2^30 bounds -> my_malloc -> zero bytes

my_realloc
  ├─ NULL ptr -> my_malloc
  ├─ validate ptr and, for standard blocks, check guard
  ├─ n == 0 -> free using configured delay behavior; return NULL
  ├─ n > 2^30 -> return NULL without changing allocation
  ├─ shrink in place; split/coalesce tail only when >= 128
  ├─ grow in place using only the following free block, if sufficient
  └─ otherwise allocate with current policy, copy old requested bytes, free old

my_heap_dump -> regions in creation order; blocks by offset; then buddy arena
my_leak_report -> allocated standard blocks only, in heap order; sum requested bytes
```

## Requirement traceability

| PDF area | Required behavior | Implementation | Existing/new verification |
|---|---|---|---|
| pp. 1-2, Part 1 | 32-byte prefix/header, block sizing, split threshold, grow, coalesce, dump | `standard_block_size`, `create_region`, `allocate_standard`, `coalesce_and_index`, `my_heap_dump` | PDF `part1`; special `first_large_region` |
| pp. 2-3, Part 2 | policy selection; heap-order ties; next-fit cursor; update after every new allocation | `first_fit`, `next_fit`, `tree_best_fit`, `tree_worst_fit`, cursor update in `allocate_standard` | four PDF policy cases; special equal-size tie cases; mixed `calloc`/realloc cursor path |
| p. 3, Part 3 | calloc overflow/zeroing; realloc null/zero/oversize, shrink/grow/move | `my_calloc`, `my_realloc`, `standard_resize_in_place` | algorithm cases `calloc`, `realloc_*`; special guard+oversize and delayed realloc |
| pp. 4-5, Part 4 | invalid/double-free distinction, guards, delayed queue/poison, leak report | `validate_standard_pointer`, `guard_is_valid`, `free_validated_standard`, `evict_oldest_delayed`, `my_leak_report` | `pointer_errors`, `overflow`, `delay*`, `leaks`; special delayed poison on eviction/lowering |
| p. 6, Part 5 | fixed buddy arena, power-of-two blocks, lowest-offset tie, XOR merge, bounds | `buddy_block_size`, `allocate_buddy`, `free_buddy_block`, buddy dump in `my_heap_dump` | `buddy`, `buddy_merge`; special max-size and oversize boundary |
| p. 7, Part 6 | O(log N) best/worst allocation and free; 100k scale; under 10 seconds | AVL free index plus embedded free nodes; performance drivers | stress best/worst workloads, 100k+ frees and allocations |
| p. 1, build rules | C17, mmap for all dynamic storage, no libc allocation, no main, output via printf | source includes and build rules | `build_tests/check_build.py` |

## Special cases covered

- Zero and maximum-sized standard requests; first request requiring a growth-sized
  region; exact split threshold and unsplittable remainder.
- `calloc` zero factors, multiplication overflow, product over 2^30, and zeroed
  payload.
- Realloc with NULL, zero, oversized request, corrupted guard, in-place shrink,
  in-place growth, movement, data preservation, and delayed-free behavior.
- Invalid stack/interior pointers; second free after coalescing (the old merged
  start is a double free, while a consumed interior header is invalid).
- Guard corruption on free/realloc; delayed block poisoning; write-after-free
  detection during normal eviction and when lowering the queue limit.
- Buddy arena uninitialized dump, exact maximum request, just-over-limit request,
  allocation exhaustion, lowest-offset ties, and full merge.
- Best/worst equal-size ties; next-fit wrap and cursor behavior across different
  allocation APIs.

The detailed cases and expected output files live in
`test_cases/algorithm_tests/expected/`; PDF examples remain in
`test_cases/pdf_tests/expected/`. The source-to-requirement map is an audit, not
a claim that untested arbitrary memory corruption is supported: the PDF excludes
header forgery and guard writes beyond the specified eight bytes.
