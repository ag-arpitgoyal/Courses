#!/usr/bin/env python3
"""Run the Part 6 workload and validate the final heap partition."""

from __future__ import annotations

import pathlib
import subprocess
import tempfile
import time


TESTER = pathlib.Path(__file__).resolve().parents[1]
ROOT = TESTER.parent
DRIVER = TESTER / "test_cases" / "stress_tests" / "performance_cases.c"
N = 100_000


def check_dump(output: str) -> tuple[int, int, int]:
    lines = output.splitlines()
    index = 0
    regions = 0
    blocks = 0
    allocated = 0
    requested = 0
    while index < len(lines):
        fields = lines[index].split()
        if len(fields) != 3 or fields[0] != "region":
            raise ValueError(f"bad region line: {lines[index]!r}")
        region_id, region_size = int(fields[1]), int(fields[2])
        if region_id != regions or region_size < 65536 or region_size % 65536:
            raise ValueError(f"invalid region header: {lines[index]!r}")
        regions += 1
        index += 1
        expected_offset = 32
        while index < len(lines) and not lines[index].startswith("region "):
            fields = lines[index].split()
            if len(fields) < 3:
                raise ValueError(f"bad block line: {lines[index]!r}")
            offset, state, size = int(fields[0]), fields[1], int(fields[2])
            if offset != expected_offset or size < 128 or size % 16:
                raise ValueError(f"invalid block partition: {lines[index]!r}")
            if state == "A" and len(fields) == 4:
                allocated += 1
                requested += int(fields[3])
            elif state == "F" and len(fields) == 3:
                pass
            else:
                raise ValueError(f"unexpected final block state: {lines[index]!r}")
            expected_offset += size
            blocks += 1
            index += 1
        if expected_offset != region_size:
            raise ValueError(
                f"region {region_id} ends at {expected_offset}, expected {region_size}"
            )
    if allocated != 2 * N or requested != 2 * N * 100:
        raise ValueError(f"final allocation totals are {allocated} blocks/{requested} bytes")
    return regions, blocks, allocated


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="a3-performance-") as temp_dir:
        executable = pathlib.Path(temp_dir) / "performance_cases"
        compile_cmd = [
            "gcc", "-std=c17", "-Wall", "-Wextra", "-Werror", "-O2",
            "-I", str(ROOT), str(ROOT / "a3.c"), str(DRIVER),
            "-o", str(executable),
        ]
        built = subprocess.run(compile_cmd, text=True, capture_output=True)
        if built.returncode:
            print("BUILD FAILED")
            print(built.stdout, end="")
            print(built.stderr, end="")
            return built.returncode

        failed = False
        for policy in ("best", "worst"):
            started = time.perf_counter()
            try:
                result = subprocess.run(
                    [str(executable), policy],
                    text=True,
                    capture_output=True,
                    timeout=10,
                )
            except subprocess.TimeoutExpired:
                print(f"FAIL {policy}: exceeded 10 seconds")
                failed = True
                continue
            elapsed = time.perf_counter() - started
            if result.returncode or result.stderr:
                print(f"FAIL {policy}: process exited {result.returncode}")
                print(result.stderr, end="")
                failed = True
                continue
            try:
                regions, blocks, allocated = check_dump(result.stdout)
            except (ValueError, IndexError) as error:
                print(f"FAIL {policy}: invalid final heap dump: {error}")
                failed = True
                continue
            if elapsed > 10:
                print(f"FAIL {policy}: {elapsed:.2f}s exceeds 10 seconds")
                failed = True
            else:
                print(
                    f"PASS {policy}: {elapsed:.2f}s, {regions} regions, "
                    f"{blocks} blocks, {allocated} allocated"
                )
        return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
