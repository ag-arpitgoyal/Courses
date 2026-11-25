#!/usr/bin/env python3
"""Tester for MTL458 Assignment 3 (Free-Space Management).

usage:  python3 run_tests.py path/to/a3.c [options]

Sections (all run by default, in this order):
  build      compiles with -std=c17, checks for forbidden functions and main
  pdf        the worked examples from the PDF, including the Part 2 table
  edge       hand-checked edge cases for every rule (expected/edge/*.txt)
  dropin     ordinary programs using your functions as malloc/free
  random     random sequences checked against a reference model of the spec
  sanitize   edge + random tests again under AddressSanitizer/UBSan
  stress     Part 6 workloads: >= 100k free blocks, >= 100k calls, 10 s limit,
             with the final heap dump and leak report checked for consistency
  scaling    checks that time per call grows like log N, not N
  ambiguous  reports how your code handles points the PDF leaves open

options:
  --only build,pdf,...   run only these sections
  --quick                fewer random seeds, skip scaling
  --seeds N              random seeds per family (default 300)
  --jobs N               parallel processes (default: CPU count)
  --keep                 keep the build directory
"""
import argparse
import concurrent.futures as futures
import difflib
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, 'models'))
import heap_model  # noqa: E402

CC = os.environ.get('CC', 'gcc')
CFLAGS = ['-std=c17', '-O2']
SECTIONS = ['build', 'pdf', 'edge', 'dropin', 'random', 'sanitize', 'stress', 'scaling', 'ambiguous']

# ---------------------------------------------------------------- output ----

COLOR = sys.stdout.isatty()


def paint(text, code):
    return f'\033[{code}m{text}\033[0m' if COLOR else text


class Results:
    def __init__(self):
        self.counts = {'PASS': 0, 'FAIL': 0, 'WARN': 0, 'INFO': 0, 'SKIP': 0}
        self.failed = []

    def add(self, status, name, detail=''):
        self.counts[status] += 1
        mark = {'PASS': paint('PASS', 32), 'FAIL': paint('FAIL', 31), 'WARN': paint('WARN', 33),
                'INFO': paint('INFO', 36), 'SKIP': paint('SKIP', 90)}[status]
        print(f'  [{mark}] {name}')
        if detail and status != 'PASS':
            for line in detail.rstrip('\n').split('\n'):
                print('         ' + line)
        if status == 'FAIL':
            self.failed.append(name)


def section(title):
    print('\n' + paint(f'== {title} ', 1) + '=' * max(0, 66 - len(title)))


def show_diff(expected, got, limit=25):
    lines = list(difflib.unified_diff(expected, got, 'expected', 'your output', lineterm='', n=2))
    if len(lines) > limit:
        lines = lines[:limit] + [f'... ({len(lines) - limit} more diff lines)']
    return '\n'.join(lines)


# ----------------------------------------------------------------- build ----

class Build:
    def __init__(self, a3_path, keep):
        self.dir = tempfile.mkdtemp(prefix='a3_tester_')
        self.keep = keep
        # Compile from a copy next to the official header, as the autograder does.
        shutil.copy(a3_path, os.path.join(self.dir, 'a3.c'))
        shutil.copy(os.path.join(HERE, 'include', 'myalloc.h'), os.path.join(self.dir, 'myalloc.h'))
        self.a3 = os.path.join(self.dir, 'a3.c')

    def path(self, name):
        return os.path.join(self.dir, name)

    def compile(self, program, out, extra=()):
        cmd = [CC, *CFLAGS, *extra, '-I', self.dir, self.a3, os.path.join(HERE, 'c', program),
               '-o', self.path(out)]
        r = subprocess.run(cmd, capture_output=True, text=True)
        return r.returncode == 0, r.stderr

    def cleanup(self):
        if self.keep:
            print(f'\nbuild directory kept: {self.dir}')
        else:
            shutil.rmtree(self.dir, ignore_errors=True)


def run(cmd, stdin=None, timeout=60):
    """Run a program; returns (stdout, stderr, returncode, seconds). Timeout -> code None."""
    start = time.monotonic()
    try:
        r = subprocess.run(cmd, input=stdin, capture_output=True, text=True, timeout=timeout,
                           errors='replace')
        return r.stdout, r.stderr, r.returncode, time.monotonic() - start
    except subprocess.TimeoutExpired as e:
        out = e.stdout.decode(errors='replace') if isinstance(e.stdout, bytes) else (e.stdout or '')
        return out, '', None, time.monotonic() - start


def crash_text(code):
    if code is None:
        return 'timed out'
    if code < 0:
        try:
            import signal
            return f'crashed ({signal.Signals(-code).name})'
        except Exception:
            return f'crashed (signal {-code})'
    return f'exit code {code}'


FORBIDDEN = {'malloc', 'calloc', 'realloc', 'free', 'brk', 'sbrk', 'strdup', 'strndup',
             'posix_memalign', 'aligned_alloc', 'memalign', 'valloc', 'pvalloc', 'reallocarray',
             'asprintf', 'vasprintf', 'open_memstream', 'getline', 'getdelim', 'realpath'}


def section_build(b, res):
    section('Build and rules')
    cmd = [CC, '-std=c17', '-Wall', '-Wextra', '-c', b.a3, '-I', b.dir, '-o', b.path('a3.o')]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        res.add('FAIL', 'compiles with -std=c17 against the official myalloc.h', r.stderr[:3000])
        return False
    res.add('PASS', 'compiles with -std=c17 against the official myalloc.h')
    warnings = [l for l in r.stderr.splitlines() if 'warning:' in l]
    if warnings:
        res.add('WARN', f'compiler warnings with -Wall -Wextra ({len(warnings)})', '\n'.join(warnings[:10]))
    else:
        res.add('PASS', 'no warnings with -Wall -Wextra')

    nm = subprocess.run(['nm', b.path('a3.o')], capture_output=True, text=True)
    if nm.returncode != 0:
        res.add('SKIP', 'symbol checks (nm not available)')
        return True
    undefined, defined = set(), {}
    for line in nm.stdout.splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[0] == 'U':
            undefined.add(parts[1])
        elif len(parts) == 3:
            defined[parts[2]] = parts[1]
    bad = sorted(undefined & FORBIDDEN)
    if bad:
        res.add('FAIL', 'uses no memory-allocating library functions',
                'calls: ' + ', '.join(bad) + '\n(all memory must come from mmap)')
    else:
        res.add('PASS', 'uses no memory-allocating library functions')
    if 'mmap' not in undefined and 'mmap64' not in undefined:
        res.add('WARN', 'calls mmap', 'mmap is never called: where does the memory come from?')
    if 'main' in defined:
        res.add('FAIL', 'does not define main', 'the starter says: Do not define main.')
    else:
        res.add('PASS', 'does not define main')
    needed = ['my_malloc', 'my_free', 'my_calloc', 'my_realloc', 'my_set_policy', 'my_set_delay',
              'my_heap_dump', 'my_leak_report']
    missing = [f for f in needed if defined.get(f) != 'T']
    if missing:
        res.add('FAIL', 'defines every function in myalloc.h', 'missing: ' + ', '.join(missing))
    else:
        res.add('PASS', 'defines every function in myalloc.h')
    extra = sorted(s for s, t in defined.items() if t in 'TDBRC' and s not in needed and not s.startswith('.'))
    if extra:
        res.add('WARN', 'helpers and globals are static',
                'non-static symbols: ' + ', '.join(extra[:12]) +
                '\nthese could clash with names in a test program; make them static')
    else:
        res.add('PASS', 'helpers and globals are static')
    source = open(b.a3, errors='replace').read()
    if re.search(r'\bstderr\b|\bfprintf\b|\bputs\s*\(|\bwrite\s*\(', source):
        res.add('WARN', 'prints only with printf',
                'the PDF says to print all output to standard output using printf')
    if 'munmap' in source:
        res.add('INFO', 'munmap is used',
                'fine for your own tables, but heap regions must never be unmapped')
    return True


# ------------------------------------------------------- expected-output ----

def compare_case(res, name, exe, arg, expected_file):
    expected = open(expected_file).read().splitlines()
    out, err, code, _ = run([exe, arg], timeout=60)
    got = out.splitlines()
    if code != 0:
        res.add('FAIL', name, crash_text(code) + '\n' + show_diff(expected, got))
    elif got != expected:
        res.add('FAIL', name, show_diff(expected, got))
    else:
        res.add('PASS', name)


PDF_CASES = [
    ('part1', 'Part 1: core allocator example'),
    ('part2', 'Part 2: placement example (first fit)'),
    ('part2_first_fit', 'Part 2 table: first fit'),
    ('part2_next_fit', 'Part 2 table: next fit'),
    ('part2_best_fit', 'Part 2 table: best fit'),
    ('part2_worst_fit', 'Part 2 table: worst fit'),
    ('part3', 'Part 3: realloc example'),
    ('part4_delay', 'Part 4: delay queue example'),
    ('part4_leak', 'Part 4: leak report example'),
    ('part5', 'Part 5: buddy example'),
]


def section_pdf(b, res):
    section('PDF examples')
    ok, err = b.compile('pdf_examples.c', 'pdf_examples')
    if not ok:
        res.add('FAIL', 'build pdf_examples.c', err[:2000])
        return
    for arg, title in PDF_CASES:
        compare_case(res, title, b.path('pdf_examples'), arg,
                     os.path.join(HERE, 'expected', 'pdf', arg + '.txt'))


def edge_case_names():
    return sorted(f[:-4] for f in os.listdir(os.path.join(HERE, 'expected', 'edge')) if f.endswith('.txt'))


def section_edge(b, res):
    section('Edge cases')
    ok, err = b.compile('edge.c', 'edge')
    if not ok:
        res.add('FAIL', 'build edge.c', err[:2000])
        return
    out, _, _, _ = run([b.path('edge'), '--list'])
    order = out.split()            # source order groups the cases by Part
    for name in order:
        compare_case(res, name, b.path('edge'), name,
                     os.path.join(HERE, 'expected', 'edge', name + '.txt'))


# ---------------------------------------------------------------- dropin ----

def section_dropin(b, res):
    section('Drop-in programs (your functions used as malloc/calloc/realloc/free)')
    ok, err = b.compile('dropin.c', 'dropin')
    if not ok:
        res.add('FAIL', 'build dropin.c', err[:2000])
        return
    for policy, label in enumerate(['first fit', 'next fit', 'best fit', 'worst fit', 'buddy']):
        out, _, code, secs = run([b.path('dropin'), str(policy)], timeout=60)
        name = f'drop-in program under {label}'
        lines = out.splitlines()
        problems = [l for l in lines if l.startswith('FAIL') or l.startswith('error:')]
        if code != 0:
            res.add('FAIL', name, crash_text(code) + '\n' + '\n'.join(lines[-10:]))
        elif 'DROPIN OK' not in lines or problems:
            res.add('FAIL', name, '\n'.join(problems[:10]) or 'did not print DROPIN OK')
        elif policy != 4 and 'leak summary: 0 blocks, 0 bytes' not in lines:
            leak = [l for l in lines if l.startswith('leak summary')]
            res.add('FAIL', name, 'everything was freed, but the leak report says: ' + (leak[0] if leak else '(nothing)'))
        else:
            res.add('PASS', f'{name} ({secs:.2f} s)')
        mis = [l for l in lines if l.startswith('MISALIGNED')]
        if mis and mis[0] != 'MISALIGNED 0':
            res.add('WARN', f'pointers are 16-byte aligned ({label})',
                    mis[0].replace('MISALIGNED', 'misaligned pointers:') +
                    '\nreal malloc returns 16-byte aligned memory; programs storing doubles may rely on it')


# ---------------------------------------------------------------- random ----

def run_family(exe, family, seeds, jobs, nops):
    """Returns (number failed, first failure (seed, ops, expected, got, code) or None)."""
    def one(seed):
        if family == 'buddy':
            ops, exp = heap_model.generate_buddy(seed, nops)
        elif family == 'no_next_fit':
            ops, exp = heap_model.generate(seed, nops, allow_next_fit=False, allow_realloc=True)
        else:
            ops, exp = heap_model.generate(seed, nops, allow_next_fit=True, allow_realloc=False)
        out, err, code, _ = run([exe], stdin='\n'.join(ops) + '\n', timeout=60)
        got = out.splitlines()
        sanitizer = 'Sanitizer' in err or 'runtime error' in err
        if code != 0 or got != exp or sanitizer:
            return (seed, ops, exp, got, code, err if sanitizer else '')
        return None
    failures = []
    with futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        for r in pool.map(one, seeds):
            if r:
                failures.append(r)
    failures.sort(key=lambda f: len(f[1]))            # report the shortest failing case
    return len(failures), (failures[0] if failures else None)


def save_failure(tag, fail):
    seed, ops, exp, got, code, err = fail
    d = os.path.join(HERE, 'failures')
    os.makedirs(d, exist_ok=True)
    base = os.path.join(d, f'{tag}_seed{seed}')
    open(base + '.ops', 'w').write('\n'.join(ops) + '\n')
    open(base + '.expected', 'w').write('\n'.join(exp) + '\n')
    open(base + '.got', 'w').write('\n'.join(got) + '\n')
    return base


def describe_random_failure(tag, fail):
    seed, ops, exp, got, code, err = fail
    base = save_failure(tag, fail)
    k = next((i for i in range(max(len(exp), len(got)))
              if i >= len(exp) or i >= len(got) or exp[i] != got[i]), None)
    text = []
    if code != 0:
        text.append(crash_text(code))
    if err:
        text.append(err.strip().split('\n')[0][:200])
    if k is not None:
        text.append(f'first difference at output line {k + 1}:')
        text.append(f'  expected: {exp[k] if k < len(exp) else "(end of output)"}')
        text.append(f'  got:      {got[k] if k < len(got) else "(end of output)"}')
    text.append(f'saved: {os.path.relpath(base)}.ops / .expected / .got')
    text.append('replay: ./driver < that .ops file (driver is built from c/driver.c; see README)')
    return '\n'.join(text)


def section_random(b, res, args):
    section('Random tests against the reference model')
    ok, err = b.compile('driver.c', 'driver')
    if not ok:
        res.add('FAIL', 'build driver.c', err[:2000])
        return
    seeds = range(1, args.seeds + 1)
    families = [
        ('no_next_fit', 'first/best/worst fit, calloc, realloc, all errors, delay queue', 500),
        ('next_fit', 'all four policies incl. next fit, all errors, delay queue (no realloc)', 500),
        ('buddy', 'buddy allocator', 600),
    ]
    for tag, title, nops in families:
        t0 = time.monotonic()
        failed, first = run_family(b.path('driver'), tag, seeds, args.jobs, nops)
        name = f'{title}: {len(seeds)} sequences of ~{nops} calls'
        if failed:
            res.add('FAIL', f'{name} ({failed} failed)', describe_random_failure(tag, first))
        else:
            res.add('PASS', f'{name} ({time.monotonic() - t0:.1f} s)')
    # a few long sequences
    long_seeds = range(10001, 10001 + max(10, args.seeds // 10))
    failed, first = run_family(b.path('driver'), 'no_next_fit', long_seeds, args.jobs, 3000)
    name = f'long sequences: {len(long_seeds)} x ~3000 calls'
    if failed:
        res.add('FAIL', f'{name} ({failed} failed)', describe_random_failure('long', first))
    else:
        res.add('PASS', name)


# -------------------------------------------------------------- sanitize ----

def section_sanitize(b, res, args):
    section('AddressSanitizer + UndefinedBehaviorSanitizer')
    flags = ['-fsanitize=address,undefined', '-fno-sanitize-recover=undefined', '-g', '-O1']
    ok, err = b.compile('driver.c', 'driver_san', flags)
    ok2, err2 = b.compile('edge.c', 'edge_san', flags)
    if not ok or not ok2:
        res.add('SKIP', 'sanitizers not available with this compiler', (err or err2)[:500])
        return
    env_bad = []
    for name in edge_case_names():
        out, errtxt, code, _ = run([b.path('edge_san'), name], timeout=120)
        if code != 0 or 'Sanitizer' in errtxt or 'runtime error' in errtxt:
            env_bad.append(f'{name}: ' + (errtxt.strip().split('\n')[0] if errtxt.strip() else crash_text(code)))
    if env_bad:
        res.add('FAIL', 'edge cases run clean under the sanitizers', '\n'.join(env_bad[:8]))
    else:
        res.add('PASS', 'edge cases run clean under the sanitizers')
    n = 40 if args.quick else 120
    for tag in ['no_next_fit', 'next_fit', 'buddy']:
        failed, first = run_family(b.path('driver_san'), tag, range(500, 500 + n), args.jobs, 400)
        name = f'random {tag.replace("_", " ")} sequences under the sanitizers ({n})'
        if failed:
            res.add('FAIL', name, describe_random_failure('sanitize_' + tag, first))
        else:
            res.add('PASS', name)


# ---------------------------------------------------------------- stress ----

def block_size(n):
    return max(128, (32 + n + 8 + 15) // 16 * 16)


def check_heap_dump(lines):
    """Check a fit-policy heap dump for internal consistency.
    Returns (problems, allocated blocks, allocated bytes)."""
    problems, count, total = [], 0, 0
    region, expect_off, last_free, region_size, expect_region = None, None, False, 0, 0
    block_re = re.compile(r'^  (\d+) ([AFD]) (\d+)(?: (\d+))?$')

    def close_region():
        if region is not None and expect_off != region_size:
            problems.append(f'region {region}: blocks end at {expect_off}, region size is {region_size}')

    for line in lines:
        if line.startswith('leak'):
            break
        m = re.match(r'^region (\d+) (\d+)$', line)
        if m:
            close_region()
            region, region_size = int(m.group(1)), int(m.group(2))
            if region != expect_region:
                problems.append(f'region numbers out of order at "{line}"')
            expect_region = region + 1
            if region_size % 65536:
                problems.append(f'region {region} size {region_size} is not a multiple of 65536')
            expect_off, last_free = 32, False
            continue
        m = block_re.match(line)
        if not m or region is None:
            problems.append(f'unexpected dump line: "{line}"')
            continue
        off, st, size = int(m.group(1)), m.group(2), int(m.group(3))
        if off != expect_off:
            problems.append(f'region {region}: block at {off}, expected one at {expect_off} (gap or overlap)')
        if size % 16 or size < 128:
            problems.append(f'block {region}:{off} has size {size} (must be a multiple of 16, >= 128)')
        if st == 'F':
            if m.group(4) is not None:
                problems.append(f'free block {region}:{off} printed with a requested size')
            if last_free:
                problems.append(f'blocks before {region}:{off} and at it are both free (not coalesced)')
            last_free = True
        else:
            if m.group(4) is None:
                problems.append(f'block {region}:{off} ({st}) printed without a requested size')
                continue
            n = int(m.group(4))
            need = block_size(n)
            if not need <= size < need + 128:
                problems.append(f'block {region}:{off} has size {size} for n = {n}: '
                                f'expected {need} (or up to {need + 127} if not split)')
            last_free = False
            if st == 'A':
                count += 1
                total += n
        expect_off = off + size
    close_region()
    return problems, count, total


def check_buddy_dump(lines):
    problems = []
    if not lines or lines[0] != 'buddy 1048576':
        return ['dump does not start with "buddy 1048576"'], 0
    blocks, off = [], 0
    for line in lines[1:]:
        m = re.match(r'^  (\d+) ([AF]) (\d+)(?: (\d+))?$', line)
        if not m:
            problems.append(f'unexpected dump line: "{line}"')
            continue
        o, st, s = int(m.group(1)), m.group(2), int(m.group(3))
        if o != off:
            problems.append(f'block at {o}, expected one at {off}')
        if s < 32 or s & (s - 1) or o % s:
            problems.append(f'block {o} size {s}: not a power of two >= 32 aligned to its size')
        if st == 'A':
            n = int(m.group(4) or 0)
            want = 32
            while want < n + 16:
                want *= 2
            if s != want:
                problems.append(f'block {o} has size {s} for n = {n}, expected {want}')
        blocks.append((o, st, s))
        off = o + s
    if off != 1048576:
        problems.append(f'blocks end at {off}, not 1048576')
    free = {(o, s) for o, st, s in blocks if st == 'F'}
    for o, s in free:
        if (o ^ s, s) in free and o < (o ^ s):
            problems.append(f'free buddies {o} and {o ^ s} (size {s}) were not merged')
    allocated = sum(1 for _, st, _ in blocks if st == 'A')
    return problems, allocated


def parse_stress_stderr(err):
    info = {}
    for line in err.splitlines():
        parts = line.split()
        if parts and parts[0] in ('EXPECT', 'PHASE2', 'PHASE2_CALLS', 'CALLS', 'CORRUPT'):
            info[parts[0]] = parts[1:]
    return info


def stress_once(b, workload, policy, holes=None, ops=None, timeout=30):
    cmd = [b.path('stress'), workload, str(policy)]
    if holes:
        cmd += [str(holes), str(ops)]
    return run(cmd, timeout=timeout)


def section_stress(b, res):
    section('Stress tests (Part 6: >= 100k free blocks, >= 100k calls, 10 s each)')
    ok, err = b.compile('stress.c', 'stress')
    if not ok:
        res.add('FAIL', 'build stress.c', err[:2000])
        return
    names = ['first fit', 'next fit', 'best fit', 'worst fit']
    workloads = [
        ('mixed', 'mixed malloc/calloc/realloc/free'),
        ('free_heavy', 'free in random order (coalescing)'),
        ('big_requests', 'requests larger than every hole'),
        ('delayed', 'mixed with my_set_delay(64)'),
    ]
    for workload, title in workloads:
        for policy, pname in enumerate(names):
            name = f'{title}, {pname}'
            out, errtxt, code, secs = stress_once(b, workload, policy)
            if code != 0:
                res.add('FAIL', name, f'{crash_text(code)} after {secs:.1f} s\n' + errtxt[-500:])
                continue
            info = parse_stress_stderr(errtxt)
            lines = out.splitlines()
            problems, count, total = check_heap_dump(lines)
            summary = [l for l in lines if l.startswith('leak summary')]
            want = f'leak summary: {info["EXPECT"][0]} blocks, {info["EXPECT"][1]} bytes' if 'EXPECT' in info else None
            if summary and want and summary[0] != want:
                problems.append(f'leak report says "{summary[0]}", the program still holds "{want}"')
            if want and (count, total) != (int(info['EXPECT'][0]), int(info['EXPECT'][1])):
                problems.append(f'heap dump shows {count} allocated blocks / {total} bytes, '
                                f'the program holds {info["EXPECT"][0]} / {info["EXPECT"][1]}')
            if info.get('CORRUPT', ['0'])[0] != '0':
                problems.append(f'{info["CORRUPT"][0]} blocks had their data corrupted')
            if problems:
                res.add('FAIL', name, '\n'.join(problems[:8]))
            elif secs > 10:
                res.add('FAIL', f'{name}: {secs:.2f} s', 'over the 10 second limit')
            elif secs > 5:
                res.add('WARN', f'{name}: {secs:.2f} s', 'under 10 s here, but close: the grader may be slower')
            else:
                res.add('PASS', f'{name}: {secs:.2f} s, {info.get("CALLS", ["?"])[0]} calls, dump consistent')
    out, errtxt, code, secs = stress_once(b, 'buddy', 4)
    name = 'buddy: 220k random malloc/free in 1 MiB'
    if code != 0:
        res.add('FAIL', name, crash_text(code))
    else:
        problems, allocated = check_buddy_dump(out.splitlines())
        info = parse_stress_stderr(errtxt)
        if 'EXPECT' in info and allocated != int(info['EXPECT'][0]):
            problems.append(f'dump shows {allocated} allocated blocks, the program holds {info["EXPECT"][0]}')
        if info.get('CORRUPT', ['0'])[0] != '0':
            problems.append('data corrupted')
        if problems:
            res.add('FAIL', name, '\n'.join(problems[:8]))
        elif secs > 10:
            res.add('FAIL', f'{name}: {secs:.2f} s', 'over the 10 second limit')
        else:
            res.add('PASS', f'{name}: {secs:.2f} s, dump consistent')


def section_scaling(b, res):
    section('Scaling: time per call as the number of free blocks grows 8x')
    if not os.path.exists(b.path('stress')):
        ok, err = b.compile('stress.c', 'stress')
        if not ok:
            res.add('FAIL', 'build stress.c', err[:2000])
            return
    # PDF: malloc O(log N) for best and worst fit; free O(log N) for every policy.
    # First and next fit may be linear, so their results are only reported.
    checks = [
        ('big_requests', 2, 'my_malloc, best fit', True),
        ('big_requests', 3, 'my_malloc, worst fit', True),
        ('free_heavy', 0, 'my_free, first fit', True),
        ('free_heavy', 2, 'my_free, best fit', True),
        ('big_requests', 0, 'my_malloc, first fit (may be linear)', False),
        ('big_requests', 1, 'my_malloc, next fit (may be linear)', False),
    ]
    ops = 100000
    for workload, policy, title, required in checks:
        per_call = []                  # microseconds per call in the timed phase
        for holes in (25000, 200000):
            best = None
            for _ in range(2):         # best of two runs, to reduce noise
                _, errtxt, code, secs = stress_once(b, workload, policy, holes, ops, timeout=60)
                if code != 0:
                    best = None
                    break
                info = parse_stress_stderr(errtxt)
                t = float(info.get('PHASE2', ['0'])[0])
                n = max(1, int(info.get('PHASE2_CALLS', [ops])[0]))
                us = t / n * 1e6
                best = us if best is None else min(best, us)
            per_call.append(best)
        if None in per_call:
            res.add('FAIL' if required else 'INFO', title, 'run failed or timed out at 200k free blocks')
            continue
        small, large = per_call
        ratio = large / max(small, 1e-3)
        text = (f'{title}: {small:.2f} us/call at 25k free blocks, '
                f'{large:.2f} us/call at 200k ({ratio:.1f}x)')
        # With O(log N) a call takes a few microseconds even at 200k free blocks
        # (it grows a little as the heap stops fitting in the CPU cache). A scan
        # over 200k blocks takes hundreds of microseconds and grows ~8x.
        if large <= 20 or ratio <= 2.5:
            res.add('PASS', text)
        elif required:
            res.add('FAIL', text, 'a call takes far longer with more free blocks: looks linear, '
                                  'the PDF requires O(log N)')
        else:
            res.add('INFO', text, 'grows with N (allowed for first/next fit, but a 10 s limit still applies)')


# ------------------------------------------------------------- ambiguous ----

AMBIGUOUS = [
    ('first_request_large', 'first request larger than 64 KiB',
     [('region 0 65536', 'region 0 stays 65536 and the request goes to region 1 (literal reading)'),
      ('region 0 131072', 'region 0 is sized by the growth rule')]),
    ('invalid_request_creates_region', 'does my_malloc(0) as the first call create region 0?',
     [('', 'no region is created'), ('region 0 65536', 'region 0 is created')]),
    ('inplace_realloc_next_fit', 'does an in-place realloc move the next-fit position?',
     [('  176 A 128 10', 'yes, the position moves to the end of the resized block'),
      ('  544 A 128 10', 'no, only new allocations move it')]),
    ('guard_check_huge_realloc', 'are guard bytes checked when realloc gets n > 2^30?',
     [('error: overflow detected', 'yes (the overflow is reported)'), ('--', 'no')]),
    ('delay_out_of_range', 'my_set_delay(-1) while 2 blocks are delayed',
     [('D 128 10', 'ignored (blocks stay delayed)'), ('F 65504', 'clamped to 0 (queue drained)')]),
]


def section_ambiguous(b, res):
    section('Points the PDF leaves open (reported, not graded here)')
    ok, err = b.compile('ambiguous.c', 'ambiguous')
    if not ok:
        res.add('FAIL', 'build ambiguous.c', err[:2000])
        return
    for arg, question, answers in AMBIGUOUS:
        out, _, code, _ = run([b.path('ambiguous'), arg])
        if code != 0:
            res.add('WARN', question, crash_text(code))
            continue
        lines = out.splitlines()
        verdict = None
        for marker, meaning in answers:
            if (marker == '' and not lines) or (marker and any(marker in l for l in lines)):
                verdict = meaning
                break
        res.add('INFO', f'{question}: {verdict or "other behaviour"}',
                '' if verdict else 'output:\n' + '\n'.join(lines[:10]))


# ------------------------------------------------------------------ main ----

def main():
    ap = argparse.ArgumentParser(description='Tester for MTL458 Assignment 3')
    ap.add_argument('a3', help='path to your a3.c')
    ap.add_argument('--only', help='comma-separated sections: ' + ','.join(SECTIONS))
    ap.add_argument('--quick', action='store_true')
    ap.add_argument('--seeds', type=int, default=None)
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 2)
    ap.add_argument('--keep', action='store_true')
    args = ap.parse_args()
    if not os.path.isfile(args.a3):
        sys.exit(f'no such file: {args.a3}')
    if args.seeds is None:
        args.seeds = 60 if args.quick else 300
    chosen = SECTIONS if not args.only else [s.strip() for s in args.only.split(',')]
    unknown = [s for s in chosen if s not in SECTIONS]
    if unknown:
        sys.exit('unknown section(s): ' + ', '.join(unknown))
    if args.quick and not args.only:
        chosen = [s for s in chosen if s != 'scaling']

    version = subprocess.run([CC, '--version'], capture_output=True, text=True).stdout.split('\n')[0]
    print(paint('MTL458 Assignment 3 tester', 1))
    print(f'testing: {os.path.abspath(args.a3)}')
    print(f'compiler: {version}')

    res = Results()
    b = Build(args.a3, args.keep)
    start = time.monotonic()
    try:
        if 'build' in chosen and not section_build(b, res):
            print('\nstopping: the file must compile before anything else can be tested')
        else:
            for name in chosen:
                if name == 'pdf': section_pdf(b, res)
                elif name == 'edge': section_edge(b, res)
                elif name == 'dropin': section_dropin(b, res)
                elif name == 'random': section_random(b, res, args)
                elif name == 'sanitize': section_sanitize(b, res, args)
                elif name == 'stress': section_stress(b, res)
                elif name == 'scaling': section_scaling(b, res)
                elif name == 'ambiguous': section_ambiguous(b, res)
    finally:
        b.cleanup()

    c = res.counts
    print('\n' + paint('== Summary ', 1) + '=' * 55)
    print(f'  {paint(str(c["PASS"]) + " passed", 32)}, {paint(str(c["FAIL"]) + " failed", 31 if c["FAIL"] else 32)}, '
          f'{c["WARN"]} warnings, {c["INFO"]} info, {c["SKIP"]} skipped   ({time.monotonic() - start:.0f} s)')
    for name in res.failed:
        print('  ' + paint('failed: ', 31) + name)
    sys.exit(1 if c['FAIL'] else 0)


if __name__ == '__main__':
    main()
