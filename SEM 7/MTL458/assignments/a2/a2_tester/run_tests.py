#!/usr/bin/env python3
"""A2 tester: build a2.c, run it over the test inputs, grade every run.

Setup: put this folder (run_tests.py, visualiser.py, samples/, cases/) next
to a2.c and workload.c. Python 3.8+, standard library only; works on
macOS and Linux.

    python3 a2_tester/run_tests.py                   everything, S -> C -> X -> Y -> U
    python3 a2_tester/run_tests.py S C               just these tiers
    python3 a2_tester/run_tests.py X08 part3 'C0*'   just these (name prefixes or patterns)
    python3 a2_tester/run_tests.py --manual C02_idle     one test, explained, with a chart
    python3 a2_tester/run_tests.py --manual my.txt       your own input file, from anywhere
    python3 a2_tester/run_tests.py --plot            Gantt chart in the terminal per case
    python3 a2_tester/run_tests.py --visualise       one HTML report of the whole run
    python3 a2_tester/run_tests.py --repeat 3        run each case 3 times; races are flaky
    python3 a2_tester/run_tests.py --list            what tests exist
    python3 a2_tester/run_tests.py --oracle my.txt   print the expected log for an input

Tiers, run in this order:
    S  samples/*.txt        the handout's six, bundled here, with their expected logs
    C  cases/C*.txt         one clarification each, beyond the samples
    X  cases/X*.txt         timing edge cases: same-instant ties, limits, long idles
    M  cases/M*.txt         mixed scenarios: several rules interacting at once
    Y  cases/Y*.txt         stress: many jobs, I/O storms, long runs
    U  cases/<other>.txt    your own tests
S is the gate: if a sample fails, later tiers are skipped (--all runs them).

Adding a test: write cases/<name>.txt in the handout's input format. An
expected log (<name>.expected) is optional; without one the built-in oracle
works it out from the handout's rules. Where no single answer exists
(events too close to order reliably on a real machine, or a job finishing
exactly on a quantum boundary, which Clarification 5 says is never tested)
the run is graded on everything except the decision order.

Every run is checked for:
  0. exact output format    one event per line, single spaces, nothing else
  1. termination            a lost SIGCONT hangs forever
  2. no leftover process    the handout requires this outright
  3. clean exit status
  4. real children          from the children's own hb/*.hb heartbeats, i.e.
                            what physically ran, not what the log claims:
                            every job used its CPU, one job at a time, and
                            none before its arrival (Clarification 9)
  5. a self-consistent log  the visualiser's replay checks
  6. decision sequence      the graded part; timestamps dropped
  7. timestamp drift        within --drift ms + 1% of t of the reference

Environment: CC=<compiler> (default: newest gcc-NN, else gcc),
A2_ROOT=<folder with a2.c and workload.c> (default: this folder's parent),
A2_BIN=<scheduler binary> to test someone else's build.
"""

import argparse
import atexit
import fnmatch
import html
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
import webbrowser

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import visualiser as viz   # noqa: E402

CASES = os.path.join(HERE, "cases")
OUTDIR = os.path.join(HERE, "out")
BIN = os.environ.get("A2_BIN")
CFLAGS = ["-std=c17", "-Wall", "-Wextra", "-O2"]

ROOT = SAMPLES = None

# The scheduler runs in a private temp folder, never in the project folder:
# workload writes hb/ into its working directory, and a synced folder
# (Google Drive, iCloud, Dropbox) can restore a stale copy of hb/ mid-run,
# which made heartbeat files vanish. a2 execs ./workload, so both binaries
# live there too, and its stdout is written here as well -- a sync stall
# while the scheduler is printing looks exactly like a hang.
RUNDIR = tempfile.mkdtemp(prefix="a2-run-")
HBDIR = os.path.join(RUNDIR, "hb")
atexit.register(shutil.rmtree, RUNDIR, True)


def set_root(path):
    """Where a2.c and workload.c live. The handout's samples are bundled in
    this folder; a samples/ next to a2.c is used only if that is missing."""
    global ROOT, SAMPLES
    ROOT = os.path.abspath(path)
    SAMPLES = os.path.join(HERE, "samples")
    if not os.path.isdir(SAMPLES):
        SAMPLES = os.path.join(ROOT, "samples")


def ensure_binaries():
    """With --no-build (or from the visualiser), use the a2 and workload
    already built in the project folder."""
    for exe in ("a2", "workload"):
        dst = os.path.join(RUNDIR, exe)
        if not os.path.exists(dst):
            src = os.path.join(ROOT, exe)
            if not os.path.exists(src):
                sys.exit("%s not built: run without --no-build, or compile it in %s" % (exe, ROOT))
            shutil.copy2(src, dst)


set_root(os.environ.get("A2_ROOT", os.path.dirname(HERE)))

TTY = sys.stdout.isatty()
GREEN, RED, YELLOW, DIM, BOLD, OFF = (
    ("\033[32m", "\033[31m", "\033[33m", "\033[2m", "\033[1m", "\033[0m") if TTY
    else ("",) * 6)


# ================================================================= oracle
#
# The expected decision log for any input, computed on paper: integer
# milliseconds under the handout's rules, clarifications and TA replies.
# It never launches a process -- it is how C/X/U cases get an answer key.
# `--check-oracle` proves it reproduces all six samples/*.expected.

def race_ms(t, la, lb):
    """How close two independent events may be before their order is luck.

    Every stop/resume adds a little lag that the children carry forward,
    while arrival and boost deadlines stay pinned to t0. So a child-timed
    event against an absolute one needs a margin that grows over the run
    (measured: ~30 ms behind by t = 3.4 s). Two child-timed events share
    most of that lag, so a flat margin does (a 16 ms gap was seen to flip).
    """
    if absolute(la) and absolute(lb):
        return 0            # both on the scheduler's own clock from t0: exact
    if absolute(la) or absolute(lb):
        return 15 + 0.015 * t
    return 25


def settled_tie(ta, la, tb, lb):
    """A job's own message landing exactly on its own turn's end is not a
    race: the rules settle the order. Clarification 1 for a finish (FINISH
    alone), and the forum reply for an I/O (IO_START, then QEXPIRE/DEMOTE).
    Only an exact tie counts -- a few ms either way really is luck."""
    if ta != tb:
        return False

    def split(label):
        for kind in ("quantum end", "finish", "io"):
            if label.endswith(" " + kind):
                return label[:-(len(kind) + 1)], kind
        return label, None

    who_a, kind_a = split(la)
    who_b, kind_b = split(lb)

    return (who_a == who_b and kind_a and kind_b
            and {kind_a, kind_b} in ({"io", "quantum end"}, {"finish", "quantum end"}))


def absolute(label):
    """Arrivals and boosts are deadlines measured from t0 by the scheduler
    itself, so two of them never race each other, even when equal."""
    return label == "boost" or label.startswith("arrival")


INF = float("inf")


class Ambiguous(Exception):
    """A situation the handout says is never tested."""


def simulate(spec):
    n, S, G = spec["n"], spec["S"], spec["G"]
    quantum, allot = spec["quantum"], spec["allot"]
    top = n - 1
    jobs = []
    for j in spec["jobs"]:
        jobs.append(dict(j, st="new", level=top, ms_left=0, qa=0,
                         cpu_left=j["cpu"], since_io=0, io_end=None))
    queues = [[] for _ in range(n)]
    log, warnings = [], []
    holder = None          # index of the job on the CPU
    was_idle = False
    done = 0
    t = 0
    next_boost = S if S > 0 else INF

    def emit(line):
        log.append("%d %s" % (t, line))

    def refill(j, lvl):
        j["level"], j["ms_left"], j["qa"] = lvl, quantum[lvl], allot[lvl]

    def holder_event_in(j):
        """ms until the running job next says something or its quantum ends,
        and which it is. FINISH beats IO beats the quantum on a tie."""
        cands = [(j["cpu_left"], 0, "finish")]
        if j["io_every"] > 0:
            cands.append((j["io_every"] - j["since_io"], 1, "io"))
        cands.append((j["ms_left"], 2, "quantum"))
        return min(cands)

    while True:
        # ---------------------------------------------- gather this instant
        msgs, timers = [], []          # (rank, idx, kind)
        sources = []                   # (time, label) for race detection

        if holder is not None:
            j = jobs[holder]
            dt, _, kind = holder_event_in(j)
            if dt == 0:
                if kind == "finish":
                    msgs.append((0, j["idx"], "finish"))
                elif kind == "io":
                    msgs.append((1, j["idx"], "io"))
                else:
                    timers.append((3, j["idx"], "quantum"))
        for j in jobs:
            if j["st"] == "blocked" and j["io_end"] == t:
                msgs.append((2, j["idx"], "rdy"))
            if j["st"] == "new" and j["arrival"] <= t:
                timers.append((5, j["idx"], "arrive"))
        if t == next_boost:
            timers.append((6, -1, "boost"))

        # ------------------------------------------------ apply, Rule 7
        for rank, idx, kind in sorted(msgs + timers):
            j = jobs[idx] if idx >= 0 else None
            if kind == "finish":
                emit("FINISH %s" % j["name"])
                queues[j["level"]].pop(0)
                j["st"] = "done"
                holder = None
                done += 1
                if done == len(jobs):
                    return log, warnings       # Clarification 10
            elif kind == "io":
                emit("IO_START %s" % j["name"])
                queues[j["level"]].pop(0)
                j["st"] = "blocked"
                j["io_end"] = t + j["io_dur"]
                j["since_io"] = 0
                if G == 0:
                    # Rule 4b: the I/O refills both counters, so a turn that
                    # ended at this very instant produces no QEXPIRE/DEMOTE
                    refill(j, j["level"])
                elif j["ms_left"] == 0:
                    # I/O exactly as the turn ends: IO_START first (Clar. 7),
                    # then the expiry, applied while the job is blocked. It
                    # joins the tail of its queue when the I/O completes.
                    lvl = j["level"]
                    j["qa"] -= 1
                    if j["qa"] > 0:
                        emit("QEXPIRE %s Q%d" % (j["name"], lvl))
                        j["ms_left"] = quantum[lvl]
                    elif lvl == 0:
                        emit("QEXPIRE %s Q0" % j["name"])
                        refill(j, 0)
                    else:
                        emit("DEMOTE %s Q%d Q%d" % (j["name"], lvl, lvl - 1))
                        refill(j, lvl - 1)
                holder = None
            elif kind == "rdy":
                emit("IO_END %s" % j["name"])
                j["st"] = "ready"
                queues[j["level"]].append(idx)
            elif kind == "quantum":
                lvl = j["level"]
                queues[lvl].pop(0)
                j["qa"] -= 1
                if j["qa"] > 0:
                    emit("QEXPIRE %s Q%d" % (j["name"], lvl))
                    j["ms_left"] = quantum[lvl]
                elif lvl == 0:                              # Clarification 8
                    emit("QEXPIRE %s Q0" % j["name"])
                    refill(j, 0)
                else:
                    emit("DEMOTE %s Q%d Q%d" % (j["name"], lvl, lvl - 1))
                    refill(j, lvl - 1)
                queues[j["level"]].append(idx)
                holder = None
            elif kind == "arrive":
                emit("ARRIVE %s" % j["name"])
                j["st"] = "ready"
                refill(j, top)
                queues[top].append(idx)
            elif kind == "boost":
                emit("BOOST")
                for q in queues:
                    q.clear()
                for k in jobs:                              # input order
                    if k["st"] in ("ready", "running", "blocked"):
                        refill(k, top)
                    if k["st"] in ("ready", "running"):
                        queues[top].append(k["idx"])
                holder = None
                next_boost += S

        # -------------------------------------------------------- dispatch
        chosen = next((q[0] for q in reversed(queues) if q), None)
        if chosen is None:
            if holder is not None:
                raise AssertionError("running job is not at the head of its queue")
            if not was_idle:
                emit("IDLE")
                was_idle = True
        elif chosen != holder:
            if holder is not None:
                jobs[holder]["st"] = "ready"            # preempted, keeps counters
            jobs[chosen]["st"] = "running"
            was_idle = False
            emit("RUN %s Q%d" % (jobs[chosen]["name"], jobs[chosen]["level"]))
        holder = chosen

        # ------------------------------------------------ next instant
        if holder is not None:
            j = jobs[holder]
            # every pending event of the running job, not just the first:
            # its I/O is timed by the child, its quantum by the scheduler
            sources.append((t + j["ms_left"], "%s quantum end" % j["name"]))
            left = j["cpu_left"]
            if j["io_every"] > 0 and j["io_every"] - j["since_io"] < left:
                sources.append((t + j["io_every"] - j["since_io"], "%s io" % j["name"]))
            else:
                sources.append((t + left, "%s finish" % j["name"]))
        for j in jobs:
            if j["st"] == "blocked":
                sources.append((j["io_end"], "%s I/O end" % j["name"]))
        pending = [j["arrival"] for j in jobs if j["st"] == "new"]
        if pending:
            first = min(pending)
            sources.append((first, "arrival at %d" % first))
        if next_boost < INF:
            sources.append((next_boost, "boost"))

        sources.sort()
        nxt = sources[0][0]
        # the event about to happen against every other pending one; later
        # pairs get their turn when the clock reaches them
        clash = [(ta, la, tb, lb) for ta, la in sources if ta == nxt
                 for tb, lb in sources if (tb, lb) != (ta, la)
                 and abs(tb - ta) < race_ms(ta, la, lb)
                 and not settled_tie(ta, la, tb, lb)]
        if clash:
            ta, la, tb, lb = clash[0]
            warnings.append("%d: %s and %s are %dms apart" % (ta, la, lb, abs(tb - ta)))

        # A job finishing exactly as its quantum ends produces a FINISH line
        # alone (Clarification 1, which names this case outright).
        # holder_event_in already ranks finish above the quantum.

        # charge the running job and advance
        if holder is not None:
            d = nxt - t
            j = jobs[holder]
            j["cpu_left"] -= d
            j["ms_left"] -= d
            if j["io_every"] > 0:
                j["since_io"] += d
        t = nxt
        if t > 600000:
            raise AssertionError("simulation ran past 600s")


def check_oracle():
    bad = 0
    for f in sorted(os.listdir(SAMPLES)):
        if f.endswith(".txt"):
            log, _ = simulate(viz.parse_input(os.path.join(SAMPLES, f)))
            with open(os.path.join(SAMPLES, f[:-4] + ".expected")) as fh:
                ok = log == [l.strip() for l in fh if l.strip()]
            bad += not ok
            print("  %s %s" % ("ok  " if ok else "DIFF", f[:-4]))
    return 1 if bad else 0


# ================================================================== build

def pick_compiler():
    """The handout wants GCC 11.1+. On a Mac, plain `gcc` is Apple clang, so
    prefer a versioned Homebrew GCC when there is one."""
    if os.environ.get("CC"):
        return os.environ["CC"]
    for v in range(20, 10, -1):
        if shutil.which("gcc-%d" % v):
            return "gcc-%d" % v
    return "gcc"


CC = pick_compiler()


def build():
    """Compile workload and a2. Warnings are shown but do not fail the run."""
    try:
        ver = subprocess.run([CC, "--version"], capture_output=True, text=True).stdout
    except FileNotFoundError:
        print("%scompiler %r not found; set CC=...%s" % (RED, CC, OFF))
        return False
    if "clang" in ver.lower():
        print("%s%s is clang, not GCC; the grader uses GCC. Set CC=gcc-NN if you have it.%s"
              % (YELLOW, CC, OFF))
    for src, out in (("workload.c", "workload"), ("a2.c", "a2")):
        path = os.path.join(ROOT, src)
        if not os.path.exists(path):
            print("%s%s not found in %s%s  (put this folder next to a2.c, or use --root)"
                  % (RED, src, ROOT, OFF))
            return False
        done = subprocess.run([CC] + CFLAGS + ["-o", os.path.join(RUNDIR, out), path],
                              capture_output=True, text=True)
        if done.returncode != 0:
            print("%sbuild failed: %s%s\n%s" % (RED, src, OFF, done.stderr))
            return False
        warnings = [l for l in done.stderr.splitlines() if ": warning:" in l]
        if warnings:
            print("%s%s: %d warning(s)%s  %s%s%s"
                  % (YELLOW, src, len(warnings), OFF, DIM,
                     warnings[0].split(": warning:")[1].strip()[:90], OFF))
    print("%sbuilt with %s %s%s" % (DIM, CC, " ".join(CFLAGS), OFF))
    return True


# =============================================================== progress

class Progress:
    """One live status line while cases run:

      ⠼ ▕██████████████▋             ▏ 52%  16/30  ✓15 ✗1  X02_16_jobs  3.1s/6.9s  eta 1m04s

    Weighted by how long each case should take (its reference log's last
    timestamp), filled in eighth-blocks so it moves smoothly, coloured
    teal -> green (turning amber once anything has failed), with a glint
    that sweeps along the filled part so a long case still looks alive.
    """

    SPIN = "⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏"
    EIGHTHS = " ▏▎▍▌▋▊▉"
    OK_RAMP = [30, 36, 37, 43, 49, 48, 47, 83]        # 256-colour teal -> green
    BAD_RAMP = [130, 136, 172, 178, 214, 220, 221, 222]  # amber, once a case failed
    WIDTH = 30

    def __init__(self, total_weight, count):
        self.total, self.count = max(total_weight, 1e-6), count
        self.done_w, self.done_n = 0.0, 0
        self.passed = self.failed = 0
        self.started = time.time()
        self.frame = 0
        self.shown = False

    def _bar(self, frac):
        cells = frac * self.WIDTH
        full = int(cells)
        part = self.EIGHTHS[int((cells - full) * 8)] if full < self.WIDTH else ""
        ramp = self.BAD_RAMP if self.failed else self.OK_RAMP
        glint = (self.frame // 2) % (full + 6) if full else -1
        out = []
        for x in range(full):
            colour = ramp[x * len(ramp) // self.WIDTH]
            out.append("\033[38;5;%dm%s" % (231 if x == glint else colour, "█"))
        if part:
            out.append("\033[38;5;%dm%s" % (ramp[min(full, self.WIDTH - 1) * len(ramp) // self.WIDTH], part))
        out.append("\033[0m" + " " * (self.WIDTH - full - (1 if part else 0)))
        return "\033[2m▕\033[0m" + "".join(out) + "\033[2m▏\033[0m"

    def draw(self, name, elapsed, weight):
        self.frame += 1
        # a case that overruns its estimate holds below 100%, not a stuck bar
        frac = min(0.99, (self.done_w + min(elapsed, weight * 0.97)) / self.total)
        spent = time.time() - self.started
        eta = (spent / frac - spent) if frac > 0.02 else None
        spin = "\033[36m%s\033[0m" % self.SPIN[self.frame % len(self.SPIN)]
        score = "\033[32m✓%d\033[0m \033[%sm✗%d\033[0m" % (
            self.passed, "31" if self.failed else "2", self.failed)
        line = "  %s %s %3d%%  %d/%d  %s  %s%s%s  %s%.1fs/%.1fs%s%s" % (
            spin, self._bar(frac), frac * 100, self.done_n + 1, self.count, score,
            BOLD, name, OFF, DIM, elapsed, weight, OFF,
            ("  eta %s" % fmt_secs(eta)) if eta is not None else "")
        sys.stdout.write("\r\033[K" + line)
        sys.stdout.flush()
        self.shown = True

    def clear(self):
        if self.shown:
            sys.stdout.write("\r\033[K")
            sys.stdout.flush()
            self.shown = False

    def finish_one(self, weight, ok=True):
        self.done_w += weight
        self.done_n += 1
        if ok:
            self.passed += 1
        else:
            self.failed += 1


def nice(path):
    """Relative when that is shorter, absolute otherwise."""
    rel = os.path.relpath(path)
    return path if rel.startswith("../..") else rel


def fmt_secs(s):
    s = int(s + 0.5)
    return "%dm%02ds" % (s // 60, s % 60) if s >= 60 else "%ds" % s


# ==================================================================== run

def survivors(pgid):
    """Any process still alive in the scheduler's process group."""
    out = subprocess.run(["ps", "-eo", "pid=,pgid=,stat=,comm="],
                         capture_output=True, text=True).stdout
    found = []
    for line in out.splitlines():
        parts = line.split(None, 3)
        if len(parts) >= 4 and parts[1] == str(pgid):
            state = "zombie" if parts[2].startswith("Z") else parts[2]
            found.append("%s(%s,%s)" % (os.path.basename(parts[3].strip()), parts[0], state))
    return found


def sweep(pgid):
    try:
        os.killpg(pgid, 9)
    except OSError:
        pass


LAST_LAUNCH_MS = None


def run_scheduler(inp, log, errlog, timeout, tick=None):
    """Run a2 on one input with stdout to `log`. Returns (verdict, detail)
    where verdict is OK, HANG, LEAK or EXIT. `tick(elapsed)` is called
    about ten times a second while it runs."""
    global LAST_LAUNCH_MS
    ensure_binaries()
    shutil.rmtree(HBDIR, ignore_errors=True)
    # workload stamps with CLOCK_MONOTONIC; read the same clock, not time.time
    LAST_LAUNCH_MS = time.clock_gettime(time.CLOCK_MONOTONIC) * 1000.0
    # stderr goes to a file, not a pipe: a leaked child inherits the pipe and
    # would keep it open, making a leak indistinguishable from a hang.
    with open(log, "w") as out, open(errlog, "w") as err:
        started = time.time()
        proc = subprocess.Popen([BIN or os.path.join(RUNDIR, "a2"), os.path.abspath(inp)],
                                stdout=out, stderr=err,
                                cwd=RUNDIR, start_new_session=True)
        pgid = proc.pid          # start_new_session: pgid == pid, even after it exits
        while proc.poll() is None:
            elapsed = time.time() - started
            if elapsed > timeout:
                left = survivors(pgid)
                sweep(pgid)
                proc.wait()
                return "HANG", "no exit after %ds; %s" % (
                    timeout, ", ".join(left[:4]) or "nothing left in its group")
            if tick:
                tick(elapsed)
            time.sleep(0.08)
        elapsed = time.time() - started

    time.sleep(0.05)             # let a just-orphaned child show up in ps
    left = survivors(pgid)
    if left:
        sweep(pgid)
        tail = open(errlog).read().strip().splitlines()
        return "LEAK", "a2 exited with status %d%s, leaving: %s" % (
            proc.returncode, (" (%s)" % tail[-1]) if tail else "", ", ".join(left[:4]))

    if proc.returncode != 0:
        tail = open(errlog).read().strip().splitlines()
        return "EXIT", "status %d%s" % (proc.returncode,
                                        (": " + tail[-1]) if tail else "")
    return "OK", "%.1fs" % elapsed


# ============================================================= heartbeats

def read_heartbeats(names):
    stamps = {}
    for name in names:
        try:
            with open(os.path.join(HBDIR, name + ".hb")) as fh:
                stamps[name] = sorted(int(l) for l in fh if l.strip())
        except (OSError, ValueError):
            stamps[name] = []
    return stamps


def ran_early(jobs, stamps, launch_ms):
    """Clarification 9: no job consumes CPU before its arrival time.

    t0 comes after the tester launched a2, so any heartbeat earlier than
    launch + arrival is certainly too early. One-sided on purpose: spawn
    cost only makes it less sensitive, never wrong.
    """
    early = []
    for j in jobs:
        st = stamps.get(j["name"], [])
        if st and j["arrival"] > 0 and st[0] < launch_ms + j["arrival"]:
            early.append("%s ran %.0f ms before its arrival"
                         % (j["name"], launch_ms + j["arrival"] - st[0]))
    return early


def starved(jobs, stamps):
    """Jobs whose heartbeats show far less CPU than they need. A program
    that prints a computed schedule without running the children lands
    here, and the handout gives that no credit."""
    short = []
    for j in jobs:
        want = j["cpu"] // 5                     # one stamp per 5 ms of CPU
        got = len(stamps.get(j["name"], []))
        if got < want // 2:
            short.append("%s %d/%d" % (j["name"], got, want))
    return short


def parallel_runs(stamps):
    """Did two children physically run at the same time?

    workload appends a timestamp to hb/<name>.hb every ~6 ms of CPU, so a
    job's stamps close together (<= 12 ms apart) mark a stretch it was
    running. Serial execution means those stretches never intersect across
    jobs; returns the milliseconds of overlap per pair, beyond a little
    slack for a switch boundary.
    """
    spans = {}
    for name, st in stamps.items():
        cur, out = None, []
        for t in st:
            if cur and t - cur[1] <= 12:
                cur[1] = t
            else:
                cur = [t, t]
                out.append(cur)
        spans[name] = [(a, b) for a, b in out if b > a]
    hits = {}
    names = sorted(spans)
    for x, a in enumerate(names):
        for b in names[x + 1:]:
            ms = sum(max(0, min(e1, e2) - max(s1, s2))
                     for s1, e1 in spans[a] for s2, e2 in spans[b])
            if ms > 20:
                hits["%s+%s" % (a, b)] = ms
    return hits


# ================================================================= format

def format_errors(inp, log):
    """The autograder compares the log line by line, so hold stdout to the
    handout's table exactly: single spaces, no padding, no stray output,
    only names from the input, only levels that exist."""
    spec = viz.parse_input(inp)
    job = "(?:%s)" % "|".join(re.escape(j["name"]) for j in spec["jobs"])
    lvl = "Q(?:%s)" % "|".join(str(i) for i in range(spec["n"]))
    line_re = re.compile(r"(0|[1-9][0-9]*) (?:ARRIVE %s|RUN %s %s|IDLE|QEXPIRE %s %s|"
                         r"DEMOTE %s %s %s|IO_START %s|IO_END %s|BOOST|FINISH %s)"
                         % (job, job, lvl, job, lvl, job, lvl, lvl, job, job, job))
    raw = open(log, "rb").read()
    try:
        text = raw.decode("ascii")
    except UnicodeDecodeError:
        return ["stdout is not plain ASCII"]
    errs = []
    if text and not text.endswith("\n"):
        errs.append("last line has no trailing newline")
    for no, line in enumerate(text.split("\n")[:-1] if text else [], 1):
        if not line_re.fullmatch(line):
            errs.append("line %d is not in the handout's format: %r" % (no, line))
            if len(errs) >= 3:
                break
    return errs


def input_warnings(spec):
    """Bounds from the handout's Assumptions. The grader never goes outside
    them, so a test that does is testing something that will not be marked."""
    w = []
    if not 1 <= spec["n"] <= 8:
        w.append("n=%d (handout: 1..8)" % spec["n"])
    if not 1 <= len(spec["jobs"]) <= 16:
        w.append("m=%d (handout: 1..16)" % len(spec["jobs"]))
    if any(not 50 <= q <= 2000 for q in spec["quantum"]):
        w.append("a quantum is outside 50..2000")
    if any(not 1 <= a <= 8 for a in spec["allot"]):
        w.append("an allotment is outside 1..8")
    if spec["S"] and not 200 <= spec["S"] <= 10000:
        w.append("S=%d (handout: 0 or 200..10000)" % spec["S"])
    names = [j["name"] for j in spec["jobs"]]
    if len(set(names)) != len(names):
        w.append("job names are not unique")
    for j in spec["jobs"]:
        if not re.fullmatch(r"[A-Za-z0-9]{1,15}", j["name"]):
            w.append("name %r (handout: letters/digits, <= 15)" % j["name"])
        if j["arrival"] > 10000:
            w.append("%s arrives after 10000" % j["name"])
        if not 50 <= j["cpu"] <= 10000:
            w.append("%s cpu=%d (handout: 50..10000)" % (j["name"], j["cpu"]))
        if j["io_every"] > 0 and j["io_dur"] < 20:
            w.append("%s io_dur < 20" % j["name"])
    return w


# ================================================================== grade

def expected_for(inp):
    """(path, None) for the reference log, or (None, why) when there is no
    single right answer. Prefers the case's own .expected file."""
    path = os.path.splitext(inp)[0] + ".expected"
    if os.path.exists(path):
        return path, None
    try:
        oracle_log, races = simulate(viz.parse_input(inp))
    except Ambiguous as exc:
        why = str(exc)
        if "finishes exactly" in why:
            why += " (Clarification 5: never tested)"
        return None, why
    if races:
        return None, "events too close to order reliably, e.g. %s" % races[0]
    os.makedirs(OUTDIR, exist_ok=True)
    path = os.path.join(OUTDIR, os.path.splitext(os.path.basename(inp))[0] + ".expected")
    with open(path, "w") as fh:
        fh.write("\n".join(oracle_log) + "\n")
    return path, None


def estimate_seconds(inp):
    """How long a case should take, for the progress bar."""
    last = 5000
    try:
        ref, _ = expected_for(inp)
        if ref:
            last = viz.parse_log(ref)[-1][0]
        else:
            last = int(simulate(viz.parse_input(inp))[0][-1].split()[0])
    except (Ambiguous, IndexError, ValueError, OSError, viz.LogError):
        pass
    return last / 1000.0 * 1.02 + 0.3


def grade(inp, log, drift_limit, strict_drift=False):
    """Everything a result line or the report needs."""
    spec = viz.parse_input(inp)
    events = viz.parse_log(log)
    run = viz.replay(spec, events)
    ref, why = expected_for(inp)
    g = {"spec": spec, "events": events, "run": run, "ref": ref, "why": why,
         "diff": None, "problems": list(run["problems"])}
    if ref is None:
        g["summary"] = "%d events, structural checks only" % len(events)
        return g
    ref_events = viz.parse_log(ref)
    diff = viz.compare(events, ref_events)
    g["diff"], g["ref_events"] = diff, ref_events
    if not diff["ok"]:
        g["problems"].insert(0, "decision %d: expected %r, got %r%s"
                             % (diff["at"] + 1, diff["expected"], diff["actual"],
                                ("  [%s]" % diff["note"]) if "note" in diff else ""))
        g["summary"] = ""
    else:
        worst = max(((abs(a[0] - b[0]) - (drift_limit + 0.01 * b[0]), a[0] - b[0], b[0])
                     for a, b in zip(events, ref_events)), default=(0, 0, 0))
        if worst[0] > 0:
            # Timestamps are not graded ("small run to run variation is
            # expected and acceptable"), and one hiccup on a busy machine can
            # put a child 100 ms behind, so this warns rather than fails.
            note = ("timestamp %+d ms off at t=%d (over %d ms + 1%%)"
                    % (worst[1], worst[2], drift_limit))
            if strict_drift:
                g["problems"].insert(0, "decisions match but a " + note)
            else:
                g["note"] = note
        g["summary"] = "%d decisions, drift %d ms" % (diff["lines"], diff["drift"])
    return g


def side_by_side(events, ref_events, at, ctx=3):
    """Lines around the first divergence: (number, expected, actual, is_it)."""
    rows = []
    for i in range(max(0, at - ctx), min(max(len(events), len(ref_events)), at + ctx + 1)):
        e = viz.fmt_decision(ref_events[i]) if i < len(ref_events) else ""
        a = viz.fmt_decision(events[i]) if i < len(events) else ""
        rows.append((i + 1, e, a, i == at))
    return rows


# ================================================================== cases

TIERS = {"S": "samples: handout behaviour", "C": "clarifications",
         "X": "timing edge cases", "M": "mixed scenarios", "Y": "stress",
         "U": "your own tests"}


def tier_of(filename, in_samples):
    if in_samples:
        return "S"
    if len(filename) > 1 and filename[0] in "CXMY" and filename[1].isdigit():
        return filename[0]
    return "U"


def descriptions():
    """name -> what the test checks, read from the table in TESTS.md."""
    out = {}
    try:
        text = open(os.path.join(HERE, "TESTS.md")).read()
    except OSError:
        return out
    for line in text.splitlines():
        m = re.match(r"\|\s*`([^`]+)`\s*\|\s*(.+?)\s*\|", line)
        if m:
            out[m.group(1)] = re.sub(r"[`*]", "", m.group(2))
    return out


def discover():
    """name -> (tier, path), in tier order."""
    found = {}
    for d in (SAMPLES, CASES):
        if os.path.isdir(d):
            for f in sorted(os.listdir(d)):
                if f.endswith(".txt"):
                    found[f[:-4]] = (tier_of(f, d == SAMPLES), os.path.join(d, f))
    return dict(sorted(found.items(), key=lambda kv: "SCXMYU".index(kv[1][0])))


def resolve_manual(query, every):
    """A test by name, partial name, or path. Returns (name, tier, path) or
    exits with what it found instead."""
    if os.path.isfile(query):
        path = os.path.abspath(query)
        name = os.path.splitext(os.path.basename(path))[0]
        return name, every.get(name, ("U", None))[0], path
    q = query[:-4] if query.endswith(".txt") else query
    hits = ([n for n in every if n == q] or [n for n in every if n.lower() == q.lower()]
            or [n for n in every if q.lower() in n.lower()])
    if len(hits) == 1:
        return hits[0], every[hits[0]][0], every[hits[0]][1]
    if not hits:
        print("%sno test called %r%s  (looked in %s/ and %s/)"
              % (RED, query, OFF, os.path.relpath(SAMPLES), os.path.relpath(CASES)))
        print("  to add it: write %s in the handout's input format;"
              % os.path.relpath(os.path.join(CASES, q + ".txt")))
        print("  %s.expected is optional, the oracle fills in otherwise." % q)
        print("  existing: " + " ".join(every))
    else:
        print("%s%r matches %d tests:%s %s" % (YELLOW, query, len(hits), OFF, " ".join(hits)))
    sys.exit(2)


def describe_manual(name, tier, path):
    """Say what was found before running, so a missing file is obvious."""
    yes, no = GREEN + "✓" + OFF, YELLOW + "✗" + OFF
    spec = viz.parse_input(path)
    print("\n  %s%s%s  (tier %s)" % (BOLD, name, OFF, tier))
    note = descriptions().get(name)
    if note:
        print("  checks    %s" % note)
    print("  input     %s %s" % (yes, nice(path)))
    print("            %s%s%s" % (DIM, viz.spec_line(spec), OFF))
    for w in input_warnings(spec):
        print("  bounds    %s outside the handout's assumptions: %s" % (no, w))
    own = os.path.splitext(path)[0] + ".expected"
    if os.path.exists(own):
        print("  expected  %s %s" % (yes, nice(own)))
    else:
        ref, why = expected_for(path)
        if ref:
            print("  expected  %s no %s; using the oracle's answer (%s)"
                  % (no, os.path.basename(own), nice(ref)))
        else:
            print("  expected  %s none possible: %s" % (no, why))
            print("            graded on everything except the decision order")
    print()


# ================================================================= report

REPORT_CSS = """
:root{--bg:#f6f7f9;--card:#fff;--ink:#1f2429;--mute:#66707a;--line:#e1e5e9;
 --ok:#2f7d4f;--okbg:#e8f4ec;--bad:#b3322e;--badbg:#fbeceb;--warn:#8a6200}
@media (prefers-color-scheme:dark){:root{--bg:#15181b;--card:#1d2125;--ink:#e6e9ec;
 --mute:#98a1aa;--line:#2e343a;--ok:#6cc58e;--okbg:#1c2e23;--bad:#f08a84;--badbg:#35201f;
 --warn:#e3bf5e}}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--ink);font:14px/1.55 ui-sans-serif,system-ui,sans-serif}
main{max-width:1280px;margin:0 auto;padding:28px 16px 60px}
h1{font-size:21px;margin:0 0 4px}
.sub{color:var(--mute);font-size:13px}
.tally{display:flex;flex-wrap:wrap;gap:10px;margin:18px 0 22px}
.tile{background:var(--card);border:1px solid var(--line);border-radius:10px;padding:10px 14px;min-width:130px}
.tile b{display:block;font-size:20px;font-variant-numeric:tabular-nums}
.tile span{color:var(--mute);font-size:12px}
h2{font-size:14px;margin:28px 0 10px;color:var(--mute);font-weight:600}
details{background:var(--card);border:1px solid var(--line);border-radius:10px;margin:8px 0}
summary{cursor:pointer;padding:10px 14px;display:flex;gap:12px;align-items:center;flex-wrap:wrap;list-style:none}
summary::-webkit-details-marker{display:none}
.pill{font:600 11px ui-monospace,Menlo,monospace;padding:2px 8px;border-radius:99px}
.pill.ok{background:var(--okbg);color:var(--ok)} .pill.bad{background:var(--badbg);color:var(--bad)}
.name{font:600 13px ui-monospace,Menlo,monospace}
.meta{color:var(--mute);font-size:12px}
.body{padding:4px 14px 14px;border-top:1px solid var(--line)}
.problems{margin:10px 0;padding-left:18px;color:var(--bad)}
.note{color:var(--warn);font-size:13px;margin:10px 0}
.chart-label{font-size:12px;color:var(--mute);margin:12px 0 4px}
.plot{background:#fff;border:1px solid var(--line);border-radius:8px;padding:8px 4px;overflow-x:auto}
table.diff{border-collapse:collapse;font:12px ui-monospace,Menlo,monospace;margin:10px 0;width:100%;max-width:760px}
table.diff td,table.diff th{padding:3px 10px;border-bottom:1px solid var(--line);text-align:left}
table.diff tr.at td{background:var(--badbg)}
table.diff th{color:var(--mute);font-weight:600}
.legend{background:#fff;color:#2b3138;border-radius:8px;padding:2px 12px 10px;display:inline-block}
.chip{display:inline-flex;align-items:center;gap:6px;margin:10px 14px 0 0;font-size:12px}
.chip i{width:14px;height:12px;border-radius:2px;display:inline-block}
.chip i.io{background:repeating-linear-gradient(45deg,#eef1f4 0 2px,#9aa7b4 2px 4px);border:1px solid #c7cfd6}
.chip i.bst{width:2px;height:14px;background:#b5762a;border-radius:0}
"""


def write_report(results, path):
    """One self-contained HTML file for the whole run: send it to anyone."""
    passed = sum(r["ok"] for r in results)
    tiers = {}
    for r in results:
        a, b = tiers.get(r["tier"], (0, 0))
        tiers[r["tier"]] = (a + r["ok"], b + 1)
    tiles = ['<div class="tile"><b>%d/%d</b><span>passed</span></div>' % (passed, len(results))]
    tiles += ['<div class="tile"><b>%d/%d</b><span>%s · %s</span></div>'
              % (a, b, t, html.escape(TIERS[t])) for t, (a, b) in tiers.items()]
    n_levels = max((r["g"]["spec"]["n"] for r in results if r["g"]), default=3)

    parts, cur = [], None
    for r in results:
        if r["tier"] != cur:
            cur = r["tier"]
            parts.append("<h2>%s · %s</h2>" % (cur, html.escape(TIERS[cur])))
        g = r["g"]
        mismatch = bool(g and g["diff"] and not g["diff"]["ok"])
        pill = ('<span class="pill ok">PASS</span>' if r["ok"] else '<span class="pill bad">%s</span>'
                % ("FAIL" if r["verdict"] == "OK" else r["verdict"]))
        body = []
        if r["problems"]:
            body.append('<ul class="problems">%s</ul>'
                        % "".join("<li>%s</li>" % html.escape(p) for p in r["problems"]))
        if g and g["ref"] is None:
            body.append('<div class="note">No exact answer for this input: %s. Graded on '
                        'everything except the decision order.</div>' % html.escape(g["why"]))
        if mismatch:
            body.append('<table class="diff"><tr><th>#</th><th>expected</th><th>your a2</th></tr>%s</table>'
                        % "".join('<tr%s><td>%d</td><td>%s</td><td>%s</td></tr>'
                                  % (' class="at"' if at else "", i, html.escape(e), html.escape(a))
                                  for i, e, a, at in side_by_side(g["events"], g["ref_events"],
                                                                  g["diff"]["at"])))
        if g and g["events"]:
            end = g["run"]["end"]
            if mismatch:
                exp_run = viz.replay(g["spec"], g["ref_events"])
                end = max(end, exp_run["end"])
                body.append('<div class="chart-label">expected</div><div class="plot">%s</div>'
                            % viz.render_chart(g["spec"], exp_run, end))
            body.append('<div class="chart-label">your a2%s</div><div class="plot">%s</div>'
                        % (" (same scale)" if mismatch else "",
                           viz.render_chart(g["spec"], g["run"], end)))
        meta = ([r["detail"]] if r["verdict"] == "OK" else []) + \
               ([g["summary"], viz.spec_line(g["spec"])] if g else [])
        parts.append('<details%s><summary>%s<span class="name">%s</span><span class="meta">%s</span>'
                     '</summary><div class="body">%s</div></details>'
                     % ("" if r["ok"] else " open", pill, html.escape(r["tag"]),
                        html.escape(" · ".join(m for m in meta if m)), "".join(body)))

    page = ("<!doctype html><html lang=en><meta charset=utf-8>"
            "<meta name=viewport content='width=device-width,initial-scale=1'>"
            "<title>A2 test report</title><style>%s</style><main>"
            "<h1>A2 scheduler test report</h1>"
            "<div class=sub>%s · %s · %s</div><div class=tally>%s</div>"
            "<div class=legend>%s</div>%s</main></html>"
            % (REPORT_CSS, html.escape(time.strftime("%Y-%m-%d %H:%M")), html.escape(CC),
               html.escape(os.uname().sysname), "".join(tiles),
               viz.legend_html(n_levels), "".join(parts)))
    with open(path, "w") as fh:
        fh.write(page)
    return path


# =================================================================== main

def print_plot(g):
    """Terminal Gantt chart; on a mismatch, expected above actual."""
    if not g or not g["events"]:
        return
    end = g["run"]["end"]
    if g["diff"] and not g["diff"]["ok"]:
        exp_run = viz.replay(g["spec"], g["ref_events"])
        end = max(end, exp_run["end"])
        print("\n       %sexpected:%s" % (DIM, OFF), end="")
        print(viz.render_ascii(g["spec"], exp_run, end=end, legend=False))
        print("       %syour a2:%s" % (DIM, OFF), end="")
    print(viz.render_ascii(g["spec"], g["run"], end=end))


def run_one(name, tier, inp, tag, args, progress, weight):
    stem = tag.replace("#", "_run")      # each repeat keeps its own log
    log = os.path.join(RUNDIR, stem + ".log")
    err = os.path.join(RUNDIR, stem + ".err")
    tick = (lambda el: progress.draw(tag, el, weight)) if progress else None
    verdict, detail = run_scheduler(inp, log, err, args.timeout, tick)
    if progress:
        progress.clear()

    problems, g = [], None
    if verdict != "OK":
        problems = ["%s: %s" % (verdict.lower(), detail)]
    else:
        problems += format_errors(inp, log)
        jobs = viz.parse_input(inp)["jobs"]
        stamps = read_heartbeats([j["name"] for j in jobs])
        short = starved(jobs, stamps)
        if short:
            problems.append("children did not get their CPU (heartbeats seen/needed): "
                            + ", ".join(short))
        early = ran_early(jobs, stamps, LAST_LAUNCH_MS)
        if early:
            problems.append("Clarification 9: " + ", ".join(early))
        both = parallel_runs(stamps)
        if both:
            problems.append("two children ran at once (heartbeats): " +
                            ", ".join("%s for %dms" % kv for kv in both.items()))
        try:
            g = grade(inp, log, args.drift, args.strict_drift)
            problems += g["problems"]
        except viz.LogError as exc:
            problems.append(str(exc))
    # keep the logs where the README says they are, now that the run is over
    for src in (log, err):
        try:
            shutil.copy2(src, os.path.join(OUTDIR, os.path.basename(src)))
        except OSError:
            pass

    return {"name": name, "tier": tier, "tag": tag, "verdict": verdict,
            "detail": detail, "problems": problems, "ok": not problems, "g": g}


def print_result(r, args):
    g = r["g"]
    if r["ok"]:
        print("  %spass%s %-30s %s%s  %s%s" % (GREEN, OFF, r["tag"], DIM, r["detail"],
                                              g["summary"] if g else "", OFF))
        if g and g.get("note"):
            print("       %s%s%s" % (YELLOW, g["note"], OFF))
    else:
        print("  %sFAIL%s %-30s %s%s%s" % (RED, OFF, r["tag"], DIM,
                                           r["detail"] if r["verdict"] == "OK" else "", OFF))
        for p in r["problems"][:4]:
            print("       %s" % p)
        if len(r["problems"]) > 4:
            print("       %s... %d more%s" % (DIM, len(r["problems"]) - 4, OFF))
        if g and g["diff"] and not g["diff"]["ok"]:
            print("       %s#    %-34s %s%s" % (DIM, "expected", "your a2", OFF))
            for i, e, a, at in side_by_side(g["events"], g["ref_events"], g["diff"]["at"], 2):
                print("       %s%-4d %-34s %s%s" % (RED if at else DIM, i, e, a, OFF))
    if args.plot:
        print_plot(g)


def main():
    ap = argparse.ArgumentParser(
        description="Build a2.c, run it over the tests, grade every run. "
                    "Run with --help-full for the whole guide.")
    ap.add_argument("cases", nargs="*", help="tiers (S C X Y U) or case names/patterns; default all")
    ap.add_argument("--manual", "-m", metavar="NAME",
                    help="one test by name, partial name or input path, explained, with a chart")
    ap.add_argument("--plot", action="store_true", help="Gantt chart in the terminal after each case")
    ap.add_argument("--visualise", "--visualize", "--viz", dest="visualise", action="store_true",
                    help="write one HTML report of the whole run and open it")
    ap.add_argument("--no-open", action="store_true", help="with --visualise, just write the file")
    ap.add_argument("--list", action="store_true", help="list the tests and exit")
    ap.add_argument("--oracle", metavar="INPUT", help="print the expected log for an input and exit")
    ap.add_argument("--check-oracle", action="store_true", help="check the oracle against samples/")
    ap.add_argument("--no-build", action="store_true")
    ap.add_argument("--all", action="store_true", help="run later tiers even if S fails")
    ap.add_argument("--repeat", type=int, default=1, help="runs per case")
    ap.add_argument("--timeout", type=int, default=90, help="seconds before a run counts as a hang")
    ap.add_argument("--drift", type=int, default=50, help="ms allowed off the reference, plus 1%% of t")
    ap.add_argument("--strict-drift", action="store_true",
                    help="fail a case whose timestamps drift past that, instead of warning")
    ap.add_argument("--root", help="folder with a2.c and workload.c")
    ap.add_argument("--help-full", action="store_true", help="the whole guide")
    args = ap.parse_args()

    if args.help_full:
        print(__doc__)
        return 0
    if args.root:
        set_root(args.root)
    os.makedirs(OUTDIR, exist_ok=True)
    every = discover()

    if args.check_oracle:
        return check_oracle()
    if args.oracle:
        try:
            log, races = simulate(viz.parse_input(args.oracle))
        except Ambiguous as exc:
            print("no single answer: %s" % exc, file=sys.stderr)
            return 2
        print("\n".join(log))
        for w in races:
            print("race: %s" % w, file=sys.stderr)
        return 0
    if args.list:
        about = descriptions()
        width = shutil.get_terminal_size((120, 20)).columns - 44
        for name, (tier, path) in every.items():
            note = about.get(name, "")
            if len(note) > width > 20:
                note = note[:width - 1].rsplit(" ", 1)[0] + "…"
            print("  %s  %-34s %s%s%s" % (tier, name, DIM, note, OFF))
        return 0

    if args.manual:
        name, tier, path = resolve_manual(args.manual, every)
        describe_manual(name, tier, path)
        chosen = [(name, tier, path)]
        args.plot = args.all = True
    else:
        if not os.path.isdir(SAMPLES):
            print("%sno samples/ in %s or %s%s" % (RED, HERE, ROOT, OFF))
            return 2
        chosen = [(n, t, p) for n, (t, p) in every.items()
                  if not args.cases or any(c.upper() == t if c.upper() in TIERS
                                           else fnmatch.fnmatch(n, c if any(ch in c for ch in "*?[")
                                                               else c + "*")
                                           for c in args.cases)]
        if not chosen:
            print("no case matches %s; try --list" % args.cases)
            return 2

    if not args.no_build and not build():
        return 2

    weights = {n: estimate_seconds(p) for n, _, p in chosen}
    progress = (Progress(sum(weights.values()) * args.repeat, len(chosen) * args.repeat)
                if TTY else None)

    results, tally, cur, skipped = [], {}, None, False
    for name, tier, inp in chosen:
        s_ok, s_all = tally.get("S", (0, 0))
        if tier != "S" and not args.all and s_ok < s_all:
            print("\n  %ssamples failing: skipping the other tiers (--all to run them)%s"
                  % (YELLOW, OFF))
            skipped = True
            break
        if tier != cur and not args.manual:
            cur = tier
            print("\n  %s%s  %s%s" % (BOLD, tier, TIERS[tier], OFF))
        for rep in range(args.repeat):
            tag = name if args.repeat == 1 else "%s#%d" % (name, rep + 1)
            r = run_one(name, tier, inp, tag, args, progress, weights[name])
            if progress:
                progress.finish_one(weights[name], r["ok"])
            results.append(r)
            a, b = tally.get(tier, (0, 0))
            tally[tier] = (a + r["ok"], b + 1)
            print_result(r, args)

    total, failures = len(results), sum(not r["ok"] for r in results)
    print("\n  " + "   ".join("%s %d/%d" % (t, a, b) for t, (a, b) in tally.items())
          + "   %stotal %d/%d passed%s" % (BOLD, total - failures, total, OFF)
          + ("  (stopped at the S gate)" if skipped else ""))

    if args.visualise and results:
        path = write_report(results, os.path.join(OUTDIR, "report.html"))
        print("  report: %s" % os.path.relpath(path))
        if not args.no_open:
            webbrowser.open("file://" + os.path.abspath(path))
    return 1 if failures or skipped else 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\n  interrupted")
        sys.exit(130)
