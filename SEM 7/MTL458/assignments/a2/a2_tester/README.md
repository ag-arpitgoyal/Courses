# A2 tester

Builds `a2.c`, runs it on every test input with the real `workload`, and grades each run.
Python 3.8+, standard library only. Works on macOS and Linux.

## Layout

```
A2/
├── a2.c                  your scheduler (the only file you submit)
├── workload.c            from the handout
└── a2_tester/
    ├── run_tests.py      the tester (run this)
    ├── visualiser.py     Gantt charts, used by run_tests.py
    ├── README.md
    ├── TESTS.md          what each individual test checks
    ├── samples/          the handout's six: part1.txt + part1.expected, ...
    ├── cases/            extra tests: <name>.txt + <name>.expected
    └── out/              created on each run: logs, report.html
```

## Run

From the `A2` folder:

```bash
python3 a2_tester/run_tests.py                 # everything
python3 a2_tester/run_tests.py S C             # only these tiers
python3 a2_tester/run_tests.py X08 part3       # only these tests (a name prefix is enough)
python3 a2_tester/run_tests.py -m C02          # one test, explained, with its Gantt chart
python3 a2_tester/run_tests.py --plot          # a Gantt chart after every test
python3 a2_tester/run_tests.py --visualise     # one HTML report of the whole run
python3 a2_tester/run_tests.py --repeat 3      # run each test 3 times (catches rare races)
python3 a2_tester/run_tests.py --list          # list every test
```

## Tests and file names

A test is an input file in the handout's format. Its **tier** comes from the start of its file name:

| Tier | File | What it tests | Graded against |
|---|---|---|---|
| S | `samples/part*.txt` | the handout's six samples | the handout's `.expected` |
| C | `cases/C<nn>_<what>.txt` | one clarification each | `.expected` |
| X | `cases/X<nn>_<what>.txt` | timing edges: same-instant ties, limits, long idles | `.expected` |
| Y | `cases/Y<nn>_<what>.txt` | stress: many jobs, heavy I/O, long runs | no expected log (see below) |
| U | `cases/<anything else>.txt` | your own tests | `.expected` if you write one |

Tiers run in the order S, C, X, Y, U. If a sample fails, the later tiers are skipped (`--all` runs them anyway).

## Add your own test

1. Write `cases/my_test.txt` in the handout's input format.
2. Optionally write `cases/my_test.expected`, one event per line, exactly as `a2` should print it.
3. Run `python3 a2_tester/run_tests.py -m my_test`.

With no `.expected` file, the tester works out the answer from the handout's rules; `--oracle cases/my_test.txt` prints it. It gives no exact answer when two events are too close together to have a reliable order on a real machine, or when a job finishes exactly as its quantum ends (Clarification 5). That run is then graded on everything except the order of events. The same applies to the Y tests.

## What each run is checked for

1. **Output format** — every line matches the handout's table: single spaces, nothing extra.
2. **Terminates** — no hang. A lost SIGCONT hangs forever; the timeout is 90 s.
3. **No leftover processes** — no child is still alive after `a2` exits.
4. **Clean exit** — exit status 0.
5. **Children really ran** — read from each child's heartbeat file: every job got its CPU time, one job at a time, never before its arrival.
6. **Log makes sense** — no RUN of a finished or blocked job, no wrong queue level, no line after the last FINISH.
7. **Order of events** — matches the expected log. This is what the autograder grades.
8. **Timing** — each timestamp within 50 ms + 1% of the expected one. This
   only warns: the handout does not compare timestamps, and one hiccup on a
   busy machine can put a child 100 ms behind. `--strict-drift` makes it fail.

## Reading a failure

```
FAIL X08_arrive_on_boost            1.7s
     decision 6: expected '500 BOOST', got '501 RUN B Q1'
     #    expected                           your a2
     5    500 ARRIVE B                       501 ARRIVE B
     6    500 BOOST                          501 RUN B Q1
```

Line 6 is the first place your output differs. Rerun that test with `-m X08` to see both Gantt charts.
Full logs are in `a2_tester/out/<test>.log` (stdout) and `.err` (stderr). With `--repeat`, each run keeps its own `<test>_runN.log`.

## Options

| Option | Effect |
|---|---|
| `--no-build` | use the existing `a2` and `workload` |
| `--timeout 90` | seconds before a run counts as hung |
| `--drift 50` | ms allowed off the expected timestamps, plus 1% of t (a warning) |
| `--strict-drift` | make that a failure instead |
| `--root DIR` | folder containing `a2.c` and `workload.c` |
| `CC=gcc-13` | compiler to use (default: the newest `gcc-NN`, else `gcc`) |
| `A2_BIN=path` | test someone else's compiled `a2` |
