# MTL458 Assignment 3 tester

Needs Linux, gcc and python3.

    python3 run_tests.py path/to/a3.c            # everything (~40 s)
    python3 run_tests.py path/to/a3.c --quick    # fewer random seeds, no scaling check
    python3 run_tests.py path/to/a3.c --only pdf,edge

Sections: build, pdf, edge, dropin, random, sanitize, stress, scaling, ambiguous.
Exit code is 0 only if nothing FAILs.

- PASS / FAIL: graded checks. WARN: likely problem worth a look.
- INFO: how your code handles points the PDF leaves open (not graded here).
- Your a3.c is compiled next to the official myalloc.h in include/, like the autograder.
- A failing random test is saved in failures/ as .ops / .expected / .got. To replay:
      gcc -std=c17 -I include your/a3.c c/driver.c -o driver && ./driver < failures/X.ops
- Expected outputs: expected/pdf (from the PDF) and expected/edge (one file per
  case in c/edge.c, each checked by hand). models/heap_model.py is the reference
  model used by the random tests.
- Stress timings depend on the machine; the 10 s limit is the PDF's.
