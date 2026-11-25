#!/usr/bin/env python3
"""Run every test in this folder against an allocator implementation.

usage:
    python3 run_tests.py                 # tests ../a3.c
    python3 run_tests.py path/to/a3.c
    python3 run_tests.py a3.c buddy      # only tests whose folder/name contains "buddy"
    python3 run_tests.py a3.c -v         # show diffs for failures

Each test is NAME.c + NAME.expected. It is compiled with
    gcc -std=c17 -O2 -I<this folder> IMPL.c NAME.c
run with a 10 second limit, and its stdout must match NAME.expected exactly.
"""
import difflib
import os
import pathlib
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor

HERE = pathlib.Path(__file__).resolve().parent
TIME_LIMIT = 10


def run_test(impl, test, build_dir):
    exe = build_dir / f"{test.parent.name}__{test.stem}"
    compiled = subprocess.run(
        ["gcc", "-std=c17", "-O2", f"-I{HERE}", str(impl), str(test), "-o", str(exe)],
        capture_output=True, text=True)
    if compiled.returncode != 0:
        return "FAIL", "compile error\n" + compiled.stderr, 0.0

    start = time.monotonic()
    try:
        result = subprocess.run([str(exe)], capture_output=True, timeout=TIME_LIMIT)
    except subprocess.TimeoutExpired:
        return "FAIL", f"timeout (more than {TIME_LIMIT}s)", float(TIME_LIMIT)
    elapsed = time.monotonic() - start
    exe.unlink(missing_ok=True)

    if result.returncode != 0:
        return "FAIL", f"crashed / exit code {result.returncode}", elapsed
    got = result.stdout.decode(errors="replace")
    want = test.with_suffix(".expected").read_text()
    if got != want:
        diff = difflib.unified_diff(want.splitlines(), got.splitlines(),
                                    "expected", "got", lineterm="", n=2)
        return "FAIL", "\n".join(list(diff)[:40]), elapsed
    return "PASS", "", elapsed


def main():
    args = [a for a in sys.argv[1:] if a != "-v"]
    verbose = "-v" in sys.argv
    impl = pathlib.Path(args[0] if args else HERE.parent / "a3.c").resolve()
    pattern = args[1] if len(args) > 1 else ""
    if not impl.exists():
        sys.exit(f"implementation not found: {impl}")

    tests = sorted(t for t in HERE.glob("*/*.c") if pattern in f"{t.parent.name}/{t.stem}")
    if not tests:
        sys.exit("no tests matched")
    print(f"testing {impl} with {len(tests)} tests\n")

    with tempfile.TemporaryDirectory() as tmp:
        with ThreadPoolExecutor(os.cpu_count() or 4) as pool:
            results = list(pool.map(lambda t: run_test(impl, t, pathlib.Path(tmp)), tests))

    per_folder = {}
    failed = 0
    for test, (status, detail, elapsed) in zip(tests, results):
        folder = per_folder.setdefault(test.parent.name, [0, 0])
        folder[1] += 1
        if status == "PASS":
            folder[0] += 1
            continue
        failed += 1
        print(f"FAIL {test.parent.name}/{test.stem} ({elapsed:.2f}s)")
        first_line = detail.splitlines()[0] if detail else ""
        print("  " + (detail.replace("\n", "\n  ") if verbose else first_line))

    print()
    for name, (passed, total) in sorted(per_folder.items()):
        print(f"  {'ok  ' if passed == total else 'FAIL'} {name:30} {passed}/{total}")
    slowest = max(zip(results, tests), key=lambda r: r[0][2])
    print(f"\nslowest: {slowest[1].parent.name}/{slowest[1].stem} {slowest[0][2]:.2f}s")
    print(f"total: {len(tests) - failed}/{len(tests)} passed")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
