# Assignment PDF tester

`run_assignment_tests.py` builds `a3.c` with `assignment_cases.c`, runs each case in a fresh process, and compares stdout byte-for-byte with the assignment PDF's examples and additional boundary/error cases. It uses only Python's standard library and GCC.

Run from the assignment directory:

```sh
python3 tests/run_assignment_tests.py
```

Run selected cases by name:

```sh
python3 tests/run_assignment_tests.py part1 realloc delay
```

Run the Part 6 best-fit and worst-fit stress workloads separately:

```sh
python3 tests/run_performance_tests.py
```

Cases included:

- `part1`: core allocation, splitting, coalescing, and growth example.
- `policy_first`, `policy_next`, `policy_best`, `policy_worst`: the Part 2 placement-policy setup and outcomes from the PDF's comparison table.
- `realloc`: the Part 3 move-then-shrink example.
- `delay`: the Part 4 delay-queue, write-after-free, double-free, and invalid-pointer example.
- `leaks`: the Part 4 leak report example.
- `buddy`: the Part 5 allocation and dump example.
- `allocation_bounds`, `calloc`, `pointer_errors`, `overflow`: invalid sizes, multiplication overflow, zero-initialization, and pointer/guard diagnostics.
- `realloc_in_place`, `realloc_move`, `realloc_zero_oversize`: in-place growth/shrink, moved data preservation, zero-size behavior, and oversized requests.
- `delay_lower`, `delay_capacity`: lowering the queue limit and filling its maximum capacity.
- `nextfit_coalesced_cursor`: next-fit selection when the cursor falls inside a newly coalesced free block.
- `buddy_merge`: coalescing buddies back into one region.

The performance runner creates 100,000 separated free blocks, performs 100,000 further allocations under best fit and worst fit, checks the final heap partition and allocation totals, and enforces the 10-second limit for each workload.

Both scripts compile in temporary directories and leave no executable in the submission folder. The suite does not alter `myalloc.h` or `a3.c`. It covers the PDF examples and selected edge/performance cases, but cannot exhaust every possible input or workload.
