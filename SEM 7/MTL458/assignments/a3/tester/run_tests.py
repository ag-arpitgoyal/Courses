#!/usr/bin/env python3
"""Run Assignment 3 test suites with a colored progress dashboard."""

from __future__ import annotations

import argparse
import os
import pathlib
import queue
import subprocess
import sys
import threading
import time


HERE = pathlib.Path(__file__).resolve().parent
CASES = HERE / "test_cases"
SUITES = {
    "build": ("Build requirements", 6),
    "pdf": ("PDF examples", 9),
    "algorithms": ("Algorithm and edge cases", 18),
    "mixed": ("Mixed integration cases", 1),
    "stress": ("Performance stress cases", 2),
}
RESET = "\033[0m"
COLORS = {
    "cyan": "\033[36m", "blue": "\033[34m", "green": "\033[32m",
    "red": "\033[31m", "yellow": "\033[33m", "dim": "\033[2m",
    "bold": "\033[1m",
}


class Dashboard:
    def __init__(self, total: int, color: bool) -> None:
        self.total = total
        self.color = color
        self.start = time.monotonic()
        self.completed = 0
        self.passed = 0
        self.suite_done = {name: 0 for name in SUITES}
        self.suite_passed = {name: 0 for name in SUITES}
        self.spinner_tick = 0

    def paint(self, value: str, color: str) -> str:
        return f"{COLORS[color]}{value}{RESET}" if self.color else value

    def box(self, title: str) -> None:
        width = 68
        label = f" {title} "
        interior = max(0, width - len(label) - 2)
        print(self.paint("┌" + label + "─" * interior + "┐", "blue"))

    def clear_progress_line(self) -> None:
        if sys.stdout.isatty():
            sys.stdout.write("\r\033[2K")
            sys.stdout.flush()

    def progress(self, current_suite: str | None = None, animate: bool = False) -> None:
        elapsed = time.monotonic() - self.start
        percent = int(self.completed * 100 / self.total) if self.total else 100
        width = 24
        filled = int(width * percent / 100)
        bar = "█" * filled + "░" * (width - filled)
        if self.completed:
            eta = elapsed / self.completed * (self.total - self.completed)
            eta_text = f"~{eta:.1f}s left"
        else:
            eta_text = "estimating time"
        current = ""
        if current_suite:
            name, total = SUITES[current_suite]
            done = self.suite_done[current_suite]
            passed = self.suite_passed[current_suite]
            category_percent = int(passed * 100 / total) if total else 100
            spinner = ""
            if animate:
                frames = "⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏"
                spinner = frames[self.spinner_tick % len(frames)] + " "
                self.spinner_tick += 1
            current = (
                f" | {spinner}{current_suite}: {passed}/{total} passed "
                f"({category_percent}%)"
            )
        line = (
            f"  {bar} {percent:3d}% | total {self.passed}/{self.total} passed"
            f" | {eta_text}{current}"
        )
        if sys.stdout.isatty():
            sys.stdout.write("\r\033[2K" + self.paint(line, "cyan"))
            sys.stdout.flush()
        else:
            print(line)

    def result_line(self, suite: str, line: str) -> bool:
        stripped = line.strip()
        self.clear_progress_line()
        if stripped.startswith("PASS ") or stripped.startswith("FAIL "):
            is_pass = stripped.startswith("PASS ")
            self.completed += 1
            self.suite_done[suite] += 1
            if is_pass:
                self.passed += 1
                self.suite_passed[suite] += 1
            color = "green" if is_pass else "red"
            print("  " + self.paint(stripped, color))
            self.progress(suite)
            return True
        if stripped:
            color = "red" if "FAILED" in stripped or stripped.startswith("ERROR") else "dim"
            print("  " + self.paint(stripped, color))
        return False


def commands() -> dict[str, list[str]]:
    python = sys.executable
    pdf_names = sorted(
        path.stem for path in (CASES / "pdf_tests" / "expected").glob("*.txt")
    )
    algorithm_names = sorted(
        path.stem for path in (CASES / "algorithm_tests" / "expected").glob("*.txt")
    )
    return {
        "build": [python, "-u", str(CASES / "build_tests" / "check_build.py")],
        "pdf": [python, "-u", str(CASES / "run_assignment_tests.py"), *pdf_names],
        "algorithms": [python, "-u", str(CASES / "run_assignment_tests.py"), *algorithm_names],
        "mixed": [python, "-u", str(CASES / "mixed_tests" / "run.py")],
        "stress": [python, "-u", str(CASES / "run_performance_tests.py")],
    }


def run_suite(name: str, command: list[str], dashboard: Dashboard) -> bool:
    title, expected = SUITES[name]
    dashboard.clear_progress_line()
    if sys.stdout.isatty():
        print()
    dashboard.box(title)
    env = os.environ.copy()
    env["PYTHONUNBUFFERED"] = "1"
    process = subprocess.Popen(
        command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, bufsize=1, env=env,
    )
    assert process.stdout is not None
    output_queue: queue.Queue[str | None] = queue.Queue()

    def read_output() -> None:
        assert process.stdout is not None
        for output_line in process.stdout:
            output_queue.put(output_line)
        output_queue.put(None)

    reader = threading.Thread(target=read_output, daemon=True)
    reader.start()
    while True:
        try:
            line = output_queue.get(timeout=0.1)
        except queue.Empty:
            if sys.stdout.isatty():
                dashboard.progress(name, animate=True)
            continue
        if line is None:
            break
        dashboard.result_line(name, line)
    reader.join()
    return_code = process.wait()

    # A build or runner failure may stop before producing one result per case.
    # Count unreported cases as failed so the denominator remains truthful.
    missing = max(0, expected - dashboard.suite_done[name])
    dashboard.completed += missing
    dashboard.suite_done[name] += missing
    if return_code != 0 and dashboard.suite_done[name] == expected:
        # A runner can report all cases but still fail its final consistency check.
        if dashboard.suite_passed[name] == expected and expected:
            dashboard.passed -= 1
            dashboard.suite_passed[name] -= 1
    dashboard.clear_progress_line()
    if sys.stdout.isatty():
        sys.stdout.write("\n")
    passed = dashboard.suite_passed[name]
    category_percent = int(passed * 100 / expected) if expected else 100
    outcome = "green" if passed == expected and return_code == 0 else "red"
    print(dashboard.paint(
        f"  └─ {passed}/{expected} passed ({category_percent}%)", outcome
    ))
    dashboard.progress()
    return passed == expected and return_code == 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--only", nargs="+", choices=tuple(SUITES),
        help="run selected suites (default: all)",
    )
    parser.add_argument("--no-color", action="store_true", help="disable ANSI colors")
    args = parser.parse_args()
    selected = args.only or list(SUITES)
    color = not args.no_color and "NO_COLOR" not in os.environ
    total = sum(SUITES[name][1] for name in selected)
    dashboard = Dashboard(total, color)

    print(dashboard.paint("╔════════════════════════════════════════════════════════════════════╗", "bold"))
    print(dashboard.paint("║             MTL458 ASSIGNMENT 3 — TEST DASHBOARD                 ║", "bold"))
    print(dashboard.paint("╚════════════════════════════════════════════════════════════════════╝", "bold"))
    print(f"  {total} checks across {len(selected)} categories")
    dashboard.progress()
    results = [
        (name, run_suite(name, commands()[name], dashboard))
        for name in selected
    ]

    elapsed = time.monotonic() - dashboard.start
    all_passed = all(passed for _, passed in results)
    print("\n" + dashboard.paint("┌─ FINAL SUMMARY ────────────────────────────────────────────────────┐", "blue"))
    for name, passed in results:
        count = dashboard.suite_passed[name]
        expected = SUITES[name][1]
        percent = int(count * 100 / expected) if expected else 100
        mark = "PASS" if passed else "FAIL"
        tone = "green" if passed else "red"
        print(dashboard.paint(
            f"│ {mark:4}  {name:12} {count:2}/{expected:<2} passed ({percent:3}%)".ljust(68) + "│",
            tone,
        ))
    summary_tone = "green" if all_passed else "red"
    print(dashboard.paint("├────────────────────────────────────────────────────────────────────┤", "blue"))
    print(dashboard.paint(
        f"│ Total: {dashboard.passed}/{total} passed | elapsed {elapsed:.1f}s".ljust(68) + "│",
        summary_tone,
    ))
    print(dashboard.paint("└────────────────────────────────────────────────────────────────────┘", "blue"))
    return 0 if all_passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
