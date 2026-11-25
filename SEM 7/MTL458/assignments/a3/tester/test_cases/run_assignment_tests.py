#!/usr/bin/env python3
"""Compile the shared case driver and compare each case with its fixture."""

from __future__ import annotations

import difflib
import pathlib
import subprocess
import sys
import tempfile


TESTER = pathlib.Path(__file__).resolve().parents[1]
ROOT = TESTER.parent
CASES = TESTER / "test_cases"
DRIVER = CASES / "pdf_tests" / "assignment_cases.c"
PDF_CASES = {
    "part1", "policy_first", "policy_next", "policy_best", "policy_worst",
    "realloc", "delay", "leaks", "buddy",
}
ALL_CASES = sorted(
    path.stem
    for folder in ("pdf_tests", "algorithm_tests")
    for path in (CASES / folder / "expected").glob("*.txt")
)


def fixture(name: str) -> str:
    folder = "pdf_tests" if name in PDF_CASES else "algorithm_tests"
    return (CASES / folder / "expected" / f"{name}.txt").read_text()


def main() -> int:
    requested = sys.argv[1:]
    names = requested or ALL_CASES
    unknown = [name for name in names if name not in ALL_CASES]
    if unknown:
        print(f"unknown test case(s): {', '.join(unknown)}", file=sys.stderr)
        print(f"available: {', '.join(ALL_CASES)}", file=sys.stderr)
        return 2

    with tempfile.TemporaryDirectory(prefix="a3-tests-") as temp_dir:
        executable = pathlib.Path(temp_dir) / "assignment_cases"
        compile_cmd = [
            "gcc", "-std=c17", "-Wall", "-Wextra", "-Werror", "-O2",
            "-I", str(ROOT), str(ROOT / "a3.c"), str(DRIVER),
            "-o", str(executable),
        ]
        built = subprocess.run(compile_cmd, text=True, capture_output=True)
        if built.returncode:
            print("BUILD FAILED", file=sys.stderr)
            sys.stderr.write(built.stdout)
            sys.stderr.write(built.stderr)
            return built.returncode

        failed = 0
        for name in names:
            result = subprocess.run([str(executable), name], text=True, capture_output=True)
            actual = result.stdout
            expected = fixture(name)
            if result.returncode == 0 and actual == expected and not result.stderr:
                print(f"PASS {name}")
                continue
            failed += 1
            print(f"FAIL {name}")
            if result.returncode:
                print(f"  exit status: {result.returncode}")
            if result.stderr:
                print("  stderr:")
                sys.stderr.write(result.stderr)
            diff = difflib.unified_diff(
                expected.splitlines(keepends=True), actual.splitlines(keepends=True),
                fromfile="expected", tofile="actual",
            )
            sys.stdout.writelines(diff)
        total = len(names)
        print(f"{total - failed}/{total} passed")
        return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
