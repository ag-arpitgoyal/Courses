# Assignment 3 tester

Run every suite from the assignment directory:

```sh
python3 tester/run_tests.py
```

Run selected suites:

```sh
python3 tester/run_tests.py --only build pdf algorithms mixed stress
```

The runner displays boxed suite headings, colored pass/fail results, category
pass percentages, an overall progress bar, total passes, and an estimated time
remaining. A spinner animates while a suite is running in an interactive
terminal. ANSI colors are used in a terminal; disable them with `--no-color` or
the `NO_COLOR` environment variable.

## Layout

```text
tester/
├── test_cases/
│   ├── pdf_tests/          # PDF examples and one expected-output file per case
│   ├── build_tests/        # C17, warnings, exported API, mmap, forbidden calls
│   ├── algorithm_tests/    # policy, buddy, realloc, guards, errors, delay edges
│   ├── mixed_tests/        # integration case combining multiple allocator features
│   └── stress_tests/       # Part 6 best-fit and worst-fit workloads
├── run_tests.py
└── Readme.md
```

The PDF and algorithm suites share a small C case driver, but each case has its
own expected output under the matching category's `expected/` folder. The build
suite checks the declared API, C17 compilation with warnings treated as errors,
absence of `main`, use of `mmap`, and absence of forbidden allocation calls. Its
symbol check accepts both Linux and macOS `nm` naming conventions.

The stress suite builds 100,000-block workloads for best fit and worst fit,
checks the final heap partition, and enforces the assignment's 10-second limit.
All test executables are created in temporary directories. The tester does not
modify `a3.c` or `myalloc.h`.
