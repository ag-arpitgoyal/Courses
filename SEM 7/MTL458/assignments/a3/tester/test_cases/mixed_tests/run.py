#!/usr/bin/env python3
from __future__ import annotations

import difflib
import pathlib
import subprocess
import tempfile


HERE = pathlib.Path(__file__).resolve().parent
TESTER = HERE.parents[1]
ROOT = TESTER.parent


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="a3-mixed-") as directory:
        executable = pathlib.Path(directory) / "mixed_cases"
        built = subprocess.run(
            ["gcc", "-std=c17", "-Wall", "-Wextra", "-Werror", "-O2",
             "-I", str(ROOT), str(ROOT / "a3.c"), str(HERE / "mixed_cases.c"),
             "-o", str(executable)], text=True, capture_output=True,
        )
        if built.returncode:
            print("FAIL mixed test build")
            print(built.stderr, end="")
            return built.returncode
        result = subprocess.run([str(executable)], text=True, capture_output=True)
        expected = (HERE / "expected.txt").read_text()
        if result.returncode == 0 and not result.stderr and result.stdout == expected:
            print("PASS mixed calloc + next-fit + delayed-free + realloc + leak-report")
            return 0
        print("FAIL mixed integration case")
        diff = difflib.unified_diff(expected.splitlines(True), result.stdout.splitlines(True),
                                    fromfile="expected", tofile="actual")
        print("".join(diff), end="")
        print(result.stderr, end="")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
