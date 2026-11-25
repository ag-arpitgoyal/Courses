#!/usr/bin/env python3
"""Check the assignment's compile, API, mmap, and forbidden-call rules."""

from __future__ import annotations

import os
import pathlib
import re
import subprocess
import tempfile


ROOT = pathlib.Path(__file__).resolve().parents[3]
REQUIRED = {
    "my_malloc", "my_free", "my_calloc", "my_realloc", "my_set_policy",
    "my_set_delay", "my_heap_dump", "my_leak_report",
}
FORBIDDEN = {
    "malloc", "calloc", "realloc", "free", "brk", "sbrk", "strdup",
    "strndup", "posix_memalign", "aligned_alloc", "memalign", "valloc",
    "pvalloc", "reallocarray", "asprintf", "vasprintf", "open_memstream",
    "getline", "getdelim", "realpath",
}


def fail(message: str) -> int:
    print(f"FAIL {message}")
    return 1


def main() -> int:
    compiler = os.environ.get("CC", "gcc")
    with tempfile.TemporaryDirectory(prefix="a3-build-") as temp:
        obj = pathlib.Path(temp) / "a3.o"
        result = subprocess.run(
            [compiler, "-std=c17", "-Wall", "-Wextra", "-Werror", "-c",
             str(ROOT / "a3.c"), "-I", str(ROOT), "-o", str(obj)],
            text=True, capture_output=True,
        )
        if result.returncode:
            return fail("compiles with -std=c17 -Wall -Wextra against myalloc.h\n" + result.stderr)
        print("PASS compiles with -std=c17 -Wall -Wextra -Werror against myalloc.h")

        source = (ROOT / "a3.c").read_text()
        if re.search(r"\bmain\s*\(", source):
            return fail("does not define main")
        print("PASS does not define main")

        nm = subprocess.run(["nm", str(obj)], text=True, capture_output=True)
        if nm.returncode:
            return fail("nm can inspect object symbols")
        undefined: set[str] = set()
        defined: dict[str, str] = {}
        for line in nm.stdout.splitlines():
            fields = line.split()
            if len(fields) == 2 and fields[0] == "U":
                undefined.add(fields[1].removeprefix("_"))
            elif len(fields) == 3:
                defined[fields[2].removeprefix("_")] = fields[1]

        forbidden = sorted(undefined & FORBIDDEN)
        if forbidden:
            return fail("uses no memory-allocating library functions: " + ", ".join(forbidden))
        print("PASS uses no memory-allocating library functions")

        if not ({"mmap", "mmap64"} & undefined):
            return fail("calls mmap for allocator-owned memory")
        print("PASS calls mmap for allocator-owned memory")

        missing = sorted(name for name in REQUIRED if defined.get(name, "").upper() != "T")
        if missing:
            return fail("defines all myalloc.h functions: " + ", ".join(missing))
        print("PASS defines all eight myalloc.h functions")

        extra = sorted(
            name for name, kind in defined.items()
            if kind in {"T", "D", "B", "R", "C"} and name not in REQUIRED
            and not name.startswith(".")
        )
        if extra:
            return fail("helpers and globals are static: " + ", ".join(extra))
        print("PASS helper functions and globals have internal linkage")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
