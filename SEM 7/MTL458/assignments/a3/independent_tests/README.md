# Independent tests for `a3.c`

Test suite for the Assignment 3 allocator (`myalloc.h`). Each test is a pair:

- `NAME.c` — a small program that calls the allocator API
- `NAME.expected` — the exact stdout it must produce

`run_tests.py` compiles every test against your `a3.c`, runs it, and compares its output.

## Requirements

- `gcc` (C17 support)
- `python3` (3.8+, no extra packages)

## Folder layout

```
a3/
├── a3.c                      # your implementation (tested by default)
└── independent_tests/
    ├── run_tests.py          # test runner
    ├── myalloc.h             # header used when compiling tests
    ├── part1_2_core_and_policies/
    ├── part3_calloc_realloc/
    ├── part4_errors_delay_leaks/
    ├── part5_buddy/
    ├── part6_perf_and_big_mixed/
    ├── parts1_3_with_big_random/
    ├── parts4_6_with_big_random/
    ├── edge_cases_all_parts/
    └── full_model_random/
```

## Running

From the `a3/` folder:

```bash
# run all tests against ../a3.c (the default)
python3 independent_tests/run_tests.py

# test a different implementation file
python3 independent_tests/run_tests.py path/to/a3.c

# run only tests whose "folder/name" contains a substring
python3 independent_tests/run_tests.py a3.c buddy
python3 independent_tests/run_tests.py a3.c part3_calloc_realloc
python3 independent_tests/run_tests.py a3.c big_first_fit

# show diffs (expected vs got) for failing tests
python3 independent_tests/run_tests.py a3.c -v
python3 independent_tests/run_tests.py a3.c buddy -v
```

Or from inside `independent_tests/`:

```bash
python3 run_tests.py              # uses ../a3.c
python3 run_tests.py ../a3.c -v
```

Note: when filtering, the implementation path must come first (the filter is the second argument).

## How each test is run

1. Compiled with:
   ```bash
   gcc -std=c17 -O2 -I independent_tests a3.c TEST.c -o TEST
   ```
2. Run with a **10 second** time limit.
3. Passes only if it exits with code 0 **and** its stdout matches `TEST.expected` exactly.

Tests run in parallel (one worker per CPU core).

## Reading the output

```
testing /.../a3/a3.c with 359 tests

FAIL part5_buddy/buddy_example (0.01s)
  @@ -3,4 +3,4 @@

  ok   edge_cases_all_parts           22/22
  FAIL part5_buddy                    32/33
  ...

slowest: part6_perf_and_big_mixed/... 1.23s
total: 358/359 passed
```

Failure reasons:

- `compile error` — `a3.c` + test didn't compile (gcc errors shown with `-v`)
- `timeout (more than 10s)` — too slow or stuck in a loop
- `crashed / exit code N` — segfault, abort, or non-zero exit
- a diff — output differs from `.expected` (full diff with `-v`, first 40 lines)

The script exits with status `0` if all tests pass, `1` otherwise.

## Debugging a single test by hand

```bash
cd a3
gcc -std=c17 -O2 -I independent_tests a3.c independent_tests/part5_buddy/buddy_example.c -o t
./t > got.txt
diff independent_tests/part5_buddy/buddy_example.expected got.txt
```

For crashes, rebuild with `-g -O0 -fsanitize=address,undefined` and run again.
