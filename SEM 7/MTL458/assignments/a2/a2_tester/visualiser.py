#!/usr/bin/env python3
"""Replay an MLFQ decision log into a timeline you can look at.

The scheduler's stdout already contains every event that changes CPU
ownership, so given the input file the whole run can be reconstructed
without the scheduler cooperating. Nothing needs to be added to a2.c, and
this works on anyone's implementation -- including samples/*.expected.

    visualiser.py a2_tester/samples/part3.txt run.log              -> timeline.html
    visualiser.py a2_tester/samples/part3.txt run.log --ascii      -> terminal view
    visualiser.py a2_tester/samples/part3.txt run.log -e a2_tester/samples/part3.expected

Or skip the log file and let it run the scheduler itself:

    visualiser.py a2_tester/samples/part3.txt --run            run ./a2, then draw it
    visualiser.py a2_tester/samples/part3.txt --run --build    recompile a2.c first

Usually you want run_tests.py --plot / --visualise / --manual instead,
which use this module for every case in a test run.

With no -e, the reference is found automatically: the .expected file next
to the input, or (with --run) the oracle's answer for it.
"""

import argparse
import html
import os
import sys
from collections import defaultdict

RUN, IO, READY = "run", "io", "ready"

# A heat ramp: cool and muted at the bottom of the hierarchy, hot at the top.
# Levels are spread across the whole ramp (see level_fill), so the top queue
# always reads as the hottest colour whether n is 2 or 8.
RAMP = ["#6b7f92", "#3d8ea6", "#2f9e87", "#6aa63f", "#c2a01f",
        "#dd8524", "#d45a35", "#bc3f4a"]


def level_fill(lvl, n):
    if n <= 1:
        return RAMP[-1]
    return RAMP[int(round(lvl * (len(RAMP) - 1) / float(n - 1)))]


def fade(hexcolor, amount=0.72):
    """Blend towards white. Waiting bars use this so a job's queue level
    stays readable even while it is off the CPU."""
    r, g, b = (int(hexcolor[i:i + 2], 16) for i in (1, 3, 5))
    return "#%02x%02x%02x" % tuple(int(c + (255 - c) * amount) for c in (r, g, b))


class LogError(Exception):
    pass


# ---------------------------------------------------------------- parsing

def parse_input(path):
    """Read the assignment's input format. Queue lines run highest-first."""
    with open(path) as fh:
        tok = fh.read().split()
    at = 0

    def take(k):
        nonlocal at
        vals = tok[at:at + k]
        if len(vals) < k:
            raise LogError("input file ended early")
        at += k
        return vals

    n, s, g = (int(v) for v in take(3))
    quantum = [int(v) for v in take(n)][::-1]   # reverse: index by level
    allot = [int(v) for v in take(n)][::-1]
    m = int(take(1)[0])

    jobs = []
    for i in range(m):
        name, arrival, cpu, io_every, io_dur = take(5)
        jobs.append({"name": name, "idx": i, "arrival": int(arrival),
                     "cpu": int(cpu), "io_every": int(io_every),
                     "io_dur": int(io_dur)})
    return {"n": n, "S": s, "G": g, "quantum": quantum,
            "allot": allot, "jobs": jobs}


def parse_log(path):
    """Turn the decision log into (time, kind, args) triples."""
    events = []
    with open(path) as fh:
        for lineno, raw in enumerate(fh, 1):
            parts = raw.split()
            if not parts:
                continue
            try:
                t = int(parts[0])
            except ValueError:
                raise LogError("%s:%d: no timestamp: %r" % (path, lineno, raw.strip()))
            events.append((t, parts[1], parts[2:], lineno))
    return events


# ------------------------------------------------------------- reconstruct

def replay(spec, events):
    """Rebuild per-job spans, idle stretches and boost marks from the log.

    Also reports anything structurally impossible -- two jobs running at
    once, a job resumed after finishing -- which catches a whole class of
    scheduler bugs before you even look at the picture.
    """
    top = spec["n"] - 1
    spans = defaultdict(list)
    idle_spans, boosts, problems = [], [], []

    level = {}
    level_hist = defaultdict(list)   # name -> [(t, level)], for colouring gaps
    running = None          # (name, level, started_at)
    idle_from = None
    blocked_from = {}
    arrived, finished = {}, {}
    last_t = 0

    def set_level(name, lvl, t):
        level[name] = lvl
        level_hist[name].append((t, lvl))

    def level_at(name, t):
        """A job's level during an interval -- the log states it only at the
        moments it changes, so the gaps have to be filled in."""
        lvl = top
        for when, val in level_hist[name]:
            if when > t:
                break
            lvl = val
        return lvl

    def ran_or_just_blocked(name, t):
        """A turn ends on the job that is running -- or on the job whose
        IO_START is the line just above, which is the I/O-exactly-as-the-
        quantum-ends case: the clarification prints IO_START first, then
        QEXPIRE/DEMOTE for that same job, still at that instant."""
        return ((running is not None and running[0] == name)
                or last_event == ("IO_START", name))

    def close_run(end):
        nonlocal running
        if running is None:
            return
        name, lvl, start = running
        if end > start:
            spans[name].append((start, end, RUN, lvl))
        running = None

    names = {j["name"]: j for j in spec["jobs"]}
    last_event = None
    all_done_at = None

    for t, kind, args, lineno in events:
        if t < last_t:
            problems.append("line %d: time goes backwards (%d after %d)" % (lineno, t, last_t))
        last_t = max(last_t, t)
        where = "line %d" % lineno
        if all_done_at is not None:
            problems.append("%s: %s printed after the last FINISH (Clarification 10)"
                            % (where, kind))
            all_done_at = None      # report once
        if args and kind not in ("IDLE", "BOOST") and args[0] not in names:
            problems.append("%s: %s names unknown job %r" % (where, kind, args[0]))
            continue

        if kind == "ARRIVE":
            name = args[0]
            if name in arrived:
                problems.append("%s: %s arrives twice" % (where, name))
            elif t + 20 < names[name]["arrival"]:
                problems.append("%s: %s arrives at %d, input says %d"
                                % (where, name, t, names[name]["arrival"]))
            arrived[name] = t
            set_level(name, top, t)

        elif kind == "RUN":
            name, lvl = args[0], int(args[1][1:])
            if name in finished:
                problems.append("%s: RUN %s after it finished" % (where, name))
            elif name not in arrived:
                problems.append("%s: RUN %s before it arrived" % (where, name))
            elif name in blocked_from:
                problems.append("%s: RUN %s while it is blocked on I/O" % (where, name))
            elif name in level and level[name] != lvl:
                problems.append("%s: RUN %s Q%d, but the log last put it at Q%d"
                                % (where, name, lvl, level[name]))
            if running is not None and running[0] != name:
                close_run(t)
            elif running is not None:
                close_run(t)        # re-dispatch of the same job
            if idle_from is not None:
                idle_spans.append((idle_from, t))
                idle_from = None
            set_level(name, lvl, t)
            running = (name, lvl, t)

        elif kind == "IDLE":
            if idle_from is not None:
                problems.append("%s: IDLE again while already idle (clarification 6)" % where)
            close_run(t)
            if idle_from is None:
                idle_from = t

        elif kind == "QEXPIRE":
            if not ran_or_just_blocked(args[0], t):
                problems.append("%s: QEXPIRE %s but it was not running" % (where, args[0]))
            set_level(args[0], int(args[1][1:]), t)

        elif kind == "DEMOTE":
            if not ran_or_just_blocked(args[0], t):
                problems.append("%s: DEMOTE %s but it was not running" % (where, args[0]))
            frm, to = int(args[1][1:]), int(args[2][1:])
            if to != frm - 1:
                problems.append("%s: DEMOTE %s skips a level" % (where, args[0]))
            if level.get(args[0], frm) != frm:
                problems.append("%s: DEMOTE %s from Q%d, but it was at Q%d"
                                % (where, args[0], frm, level[args[0]]))
            set_level(args[0], to, t)

        elif kind == "IO_START":
            name = args[0]
            if running is None or running[0] != name:
                problems.append("%s: IO_START %s but it was not running" % (where, name))
            close_run(t)
            blocked_from[name] = t

        elif kind == "IO_END":
            name = args[0]
            start = blocked_from.pop(name, None)
            if start is None:
                problems.append("%s: IO_END %s without a matching IO_START" % (where, name))
            else:
                spans[name].append((start, t, IO, level_at(name, start)))

        elif kind == "BOOST":
            boosts.append(t)
            for name in list(level):
                if name not in finished:
                    set_level(name, top, t)

        elif kind == "FINISH":
            name = args[0]
            if running is not None and running[0] == name:
                close_run(t)
            elif name in finished:
                problems.append("%s: %s finishes twice" % (where, name))
            else:
                problems.append("%s: FINISH %s but it was not running" % (where, name))
            finished[name] = t
            if len(finished) == len(names):
                all_done_at = t

        else:
            problems.append("%s: unknown event %r" % (where, kind))

        last_event = (kind, args[0] if args else None)

    unfinished = [j["name"] for j in spec["jobs"] if j["name"] not in finished]
    if unfinished:
        problems.append("never finished: %s" % ", ".join(unfinished))

    close_run(last_t)
    if idle_from is not None:
        idle_spans.append((idle_from, last_t))

    # Whatever is left between arrival and finish is time spent waiting in a
    # ready queue -- the part the log never states outright.
    for job in spec["jobs"]:
        name = job["name"]
        if name not in arrived:
            continue
        end = finished.get(name, last_t)
        busy = sorted((s, e) for s, e, _, _ in spans[name])
        cursor = arrived[name]
        gaps = []
        for s, e in busy:
            if s > cursor:
                gaps.append((cursor, s))
            cursor = max(cursor, e)
        if cursor < end:
            gaps.append((cursor, end))
        for s, e in gaps:
            spans[name].append((s, e, READY, level_at(name, s)))
        spans[name].sort()

    overlaps = find_overlaps(spans)
    problems.extend(overlaps)

    return {"spans": spans, "idle": idle_spans, "boosts": boosts,
            "arrived": arrived, "finished": finished,
            "end": last_t, "problems": problems}


def find_overlaps(spans):
    """Exactly one job may hold the CPU at a time. Prove it from the log."""
    running = []
    for name, items in spans.items():
        running.extend((s, e, name) for s, e, kind, _ in items if kind == RUN)
    running.sort()
    bad = []
    for (s1, e1, n1), (s2, e2, n2) in zip(running, running[1:]):
        if s2 < e1:
            bad.append("%s and %s both running over [%d, %d)" % (n1, n2, s2, min(e1, e2)))
    return bad


# ----------------------------------------------------------------- compare

def decisions(events):
    """The graded part of a log: the sequence, with timestamps dropped."""
    return [(kind, tuple(args)) for _, kind, args, _ in events]


def compare(actual, expected):
    a, b = decisions(actual), decisions(expected)
    for i, (x, y) in enumerate(zip(a, b)):
        if x != y:
            return {"ok": False, "at": i,
                    "actual": fmt_decision(actual[i]),
                    "expected": fmt_decision(expected[i])}
    if len(a) != len(b):
        longer, which = (actual, "actual") if len(a) > len(b) else (expected, "expected")
        return {"ok": False, "at": min(len(a), len(b)),
                "actual": "(%d lines)" % len(a),
                "expected": "(%d lines)" % len(b),
                "note": "%s has %d extra line(s), first is %r"
                        % (which, abs(len(a) - len(b)),
                           fmt_decision(longer[min(len(a), len(b))]))}

    drift = max((abs(x[0] - y[0]) for x, y in zip(actual, expected)), default=0)
    return {"ok": True, "drift": drift, "lines": len(a)}


def fmt_decision(event):
    t, kind, args, _ = event
    return " ".join([str(t), kind] + list(args))


# ------------------------------------------------------------------ ascii

ANSI_RAMP = [67, 31, 36, 71, 178, 172, 167, 131]     # 256-colour cousins of RAMP


def render_ascii(spec, run, width=None, colour=None, end=None, legend=True):
    """Terminal Gantt chart. Fits the terminal and colours each running
    stretch by queue level when writing to a tty."""
    import shutil as _sh
    if colour is None:
        colour = sys.stdout.isatty()
    pad = max(len(j["name"]) for j in spec["jobs"])
    if width is None:
        width = max(40, min(160, _sh.get_terminal_size((120, 20)).columns - pad - 8))
    end = max(end or run["end"], 1)
    scale = width / end
    out = ["", "  %d queues   S=%s   G=%s   span=%dms"
           % (spec["n"], spec["S"] or "off", spec["G"], end), ""]

    def paint(ch):
        if not colour or not ch.isdigit():
            return ch
        lvl = int(ch)
        idx = int(round(lvl * (len(ANSI_RAMP) - 1) / float(max(spec["n"] - 1, 1)))) \
            if spec["n"] > 1 else len(ANSI_RAMP) - 1
        return "\033[48;5;%dm\033[97m%s\033[0m" % (ANSI_RAMP[idx], ch)

    for job in spec["jobs"]:
        name = job["name"]
        row = [" "] * width
        for s, e, kind, lvl in run["spans"][name]:
            lo, hi = int(s * scale), max(int(e * scale), int(s * scale) + 1)
            glyph = str(lvl) if kind == RUN else ("~" if kind == IO else ".")
            for x in range(lo, min(hi, width)):
                row[x] = glyph
        out.append("  %-*s |%s|" % (pad, name, "".join(paint(c) for c in row)))

    marks = [" "] * width
    for t in run["boosts"]:
        x = min(int(t * scale), width - 1)
        marks[x] = "B"
    for s, e in run["idle"]:
        for x in range(int(s * scale), min(max(int(e * scale), int(s * scale) + 1), width)):
            if marks[x] == " ":
                marks[x] = "-"
    ruler = [" "] * width
    step = tick_step(end)
    for t in range(0, end + 1, step):
        x = int(t * scale)
        label = str(t)
        if x + len(label) <= width and all(c == " " for c in ruler[max(0, x - 1):x + len(label)]):
            ruler[x:x + len(label)] = label
    out += ["  %-*s |%s|" % (pad, "", "".join(marks)),
            "  %-*s  %s  ms" % (pad, "", "".join(ruler))]
    if legend:
        out += ["", "  digit = running at that queue level    ~ = blocked on I/O",
                "  .     = waiting in a ready queue       - = cpu idle    B = boost", ""]
    return "\n".join(out)


# ------------------------------------------------------------------- html

def render_html(spec, run, diff, title):
    return PAGE % {
        "title": html.escape(title),
        "meta": html.escape(spec_line(spec)),
        "svg": render_chart(spec, run),
        "legend": legend_html(spec["n"]),
        "report": report_html(run, diff),
    }


def spec_line(spec):
    return ("%d queues · quanta %s · allotments %s · S=%s · G=%d · %d jobs"
            % (spec["n"], spec["quantum"][::-1], spec["allot"][::-1],
               spec["S"] or "off", spec["G"], len(spec["jobs"])))


def render_chart(spec, run, end=None):
    """The Gantt chart alone, as an <svg>. Pass the same `end` to two
    charts to draw them at the same scale, e.g. expected over actual."""
    end = max(end or run["end"], 1)
    px = max(0.06, min(1.2, 1100.0 / end))
    gutter, row_h, bar_h, top_pad = 130, 34, 20, 34
    plot_w = end * px
    width = gutter + plot_w + 30
    height = top_pad + row_h * len(spec["jobs"]) + 46

    def x_of(t):
        return gutter + t * px

    # Explicit pixel width, so a long run scrolls inside .plot at a readable
    # scale instead of being squeezed into the page.
    svg = ['<svg viewBox="0 0 %.0f %.0f" width="%.0f" height="%.0f" '
           'xmlns="http://www.w3.org/2000/svg" font-family="ui-monospace,Menlo,monospace">'
           % (width, height, width, height),
           '<defs><pattern id="io" width="6" height="6" patternUnits="userSpaceOnUse" '
           'patternTransform="rotate(45)"><rect width="6" height="6" fill="#eef1f4"/>'
           '<line x1="0" y1="0" x2="0" y2="6" stroke="#9aa7b4" stroke-width="2"/>'
           '</pattern></defs>']

    for s, e in run["idle"]:
        svg.append('<rect x="%.1f" y="%d" width="%.1f" height="%d" fill="#e9eef2"/>'
                   % (x_of(s), top_pad - 8, max((e - s) * px, 1), row_h * len(spec["jobs"]) + 8))

    step = tick_step(end)
    for t in range(0, end + 1, step):
        svg.append('<line x1="%.1f" y1="%d" x2="%.1f" y2="%d" stroke="#e3e7ea" stroke-width="1"/>'
                   % (x_of(t), top_pad - 8, x_of(t), top_pad + row_h * len(spec["jobs"])))
        svg.append('<text x="%.1f" y="%d" font-size="10" fill="#8a949e" text-anchor="middle">%d</text>'
                   % (x_of(t), top_pad - 14, t))

    for i, job in enumerate(spec["jobs"]):
        name = job["name"]
        y = top_pad + i * row_h
        svg.append('<text x="%d" y="%d" font-size="12" fill="#2b3138" text-anchor="end">%s</text>'
                   % (gutter - 12, y + bar_h - 5, html.escape(name)))
        svg.append('<line x1="%d" y1="%.1f" x2="%.1f" y2="%.1f" stroke="#eceff1" stroke-width="1"/>'
                   % (gutter, y + bar_h / 2.0, x_of(end), y + bar_h / 2.0))

        for s, e, kind, lvl in run["spans"][name]:
            w = max((e - s) * px, 1.2)
            if kind == RUN:
                fill = level_fill(lvl, spec["n"])
                svg.append('<rect x="%.1f" y="%d" width="%.1f" height="%d" rx="2" fill="%s">'
                           '<title>%s  Q%d  %d-%dms</title></rect>'
                           % (x_of(s), y, w, bar_h, fill, html.escape(name), lvl, s, e))
            elif kind == IO:
                svg.append('<rect x="%.1f" y="%d" width="%.1f" height="%d" rx="2" '
                           'fill="url(#io)" stroke="#c7cfd6" stroke-width="0.5">'
                           '<title>%s blocked on I/O  %d-%dms</title></rect>'
                           % (x_of(s), y + 3, w, bar_h - 6, html.escape(name), s, e))
            else:
                svg.append('<rect x="%.1f" y="%.1f" width="%.1f" height="5" rx="1" fill="%s">'
                           '<title>%s waiting at Q%d  %d-%dms</title></rect>'
                           % (x_of(s), y + bar_h / 2.0 - 2.5, w,
                              fade(level_fill(lvl, spec["n"])),
                              html.escape(name), lvl, s, e))

        fin = run["finished"].get(name)
        if fin is not None:
            svg.append('<line x1="%.1f" y1="%d" x2="%.1f" y2="%d" stroke="#2b3138" stroke-width="2"/>'
                       % (x_of(fin), y - 2, x_of(fin), y + bar_h + 2))

    for t in run["boosts"]:
        svg.append('<line x1="%.1f" y1="%d" x2="%.1f" y2="%d" stroke="#b5762a" '
                   'stroke-width="1.5" stroke-dasharray="4 3"><title>boost at %dms</title></line>'
                   % (x_of(t), top_pad - 8, x_of(t), top_pad + row_h * len(spec["jobs"]), t))
    svg.append("</svg>")
    return "\n".join(svg)


def tick_step(end):
    for step in (100, 200, 250, 500, 1000, 2000, 5000):
        if end / step <= 14:
            return step
    return 10000


def legend_html(n):
    chips = []
    for lvl in range(n - 1, -1, -1):
        chips.append('<span class="chip"><i style="background:%s"></i>Q%d running</span>'
                     % (level_fill(lvl, n), lvl))
    chips.append('<span class="chip"><i class="io"></i>blocked on I/O</span>')
    chips.append('<span class="chip"><i style="background:%s"></i>waiting</span>'
                 % fade(level_fill(n - 1, n)))
    chips.append('<span class="chip"><i style="background:#e9eef2"></i>cpu idle</span>')
    chips.append('<span class="chip"><i class="bst"></i>boost</span>')
    return "".join(chips)


def report_html(run, diff):
    out = []
    if run["problems"]:
        items = "".join("<li>%s</li>" % html.escape(p) for p in run["problems"])
        out.append('<div class="box bad"><b>Log is not self-consistent</b><ul>%s</ul></div>' % items)

    if diff is None:
        return "".join(out)

    if diff["ok"]:
        out.append('<div class="box good"><b>Decisions match</b> — all %d lines, '
                   'worst timestamp drift %d ms.</div>' % (diff["lines"], diff["drift"]))
    else:
        note = ("<div>%s</div>" % html.escape(diff["note"])) if "note" in diff else ""
        out.append('<div class="box bad"><b>First divergence at decision %d</b>'
                   '<table><tr><td>expected</td><td><code>%s</code></td></tr>'
                   '<tr><td>actual</td><td><code>%s</code></td></tr></table>%s</div>'
                   % (diff["at"] + 1, html.escape(diff["expected"]),
                      html.escape(diff["actual"]), note))
    return "".join(out)


PAGE = """<!doctype html>
<meta charset="utf-8"><title>%(title)s</title>
<style>
 body{font:14px/1.6 ui-sans-serif,system-ui,sans-serif;color:#2b3138;margin:28px auto;max-width:1240px;padding:0 20px}
 h1{font-size:19px;font-weight:600;margin:0 0 2px}
 .meta{color:#71797f;font-size:13px;margin-bottom:20px}
 .plot{border:1px solid #e3e7ea;border-radius:8px;padding:10px 6px;overflow-x:auto}
 .chip{display:inline-flex;align-items:center;gap:6px;margin:12px 16px 0 0;font-size:12px;color:#5a6169}
 .chip i{width:14px;height:12px;border-radius:2px;display:inline-block}
 .chip i.io{background:repeating-linear-gradient(45deg,#eef1f4 0 2px,#9aa7b4 2px 4px);border:1px solid #c7cfd6}
 .chip i.bst{width:2px;height:14px;background:#b5762a;border-radius:0}
 .box{margin-top:18px;padding:12px 14px;border-radius:8px;font-size:13px}
 .box.good{background:#eef6ef;border:1px solid #cfe3d2}
 .box.bad{background:#fbeeee;border:1px solid #f0d2d2}
 .box ul{margin:6px 0 0 18px;padding:0}
 .box table{margin-top:6px;border-collapse:collapse}
 .box td{padding:2px 12px 2px 0;vertical-align:top}
 code{font:12px ui-monospace,Menlo,monospace;background:#fff;padding:1px 5px;border-radius:3px;border:1px solid #e3e7ea}
</style>
<h1>%(title)s</h1>
<div class="meta">%(meta)s</div>
<div class="plot">%(svg)s</div>
<div>%(legend)s</div>
%(report)s
"""


# ------------------------------------------------------------------- main

def run_first(args):
    """--run: build (optionally) and run ./a2 on the input, returning the
    path of the captured log. Uses the tester's runner so a hang or leaked
    child is caught the same way."""
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import run_tests

    os.makedirs(run_tests.OUTDIR, exist_ok=True)
    if args.build and not run_tests.build():
        raise LogError("build failed")
    stem = os.path.basename(args.input).rsplit(".", 1)[0]
    log = os.path.join(run_tests.OUTDIR, stem + ".log")
    err = os.path.join(run_tests.OUTDIR, stem + ".err")
    verdict, detail = run_tests.run_scheduler(os.path.abspath(args.input), log, err,
                                              args.timeout)
    print("a2: %s (%s)  log: %s" % (verdict.lower(), detail, os.path.relpath(log)),
          file=sys.stderr)
    if not args.expected:
        args.expected, _ = run_tests.expected_for(os.path.abspath(args.input))
    if args.out is None:
        args.out = os.path.join(run_tests.OUTDIR, stem + ".html")
    return log, verdict


def main(argv=None):
    ap = argparse.ArgumentParser(description="Replay an MLFQ decision log into a timeline.")
    ap.add_argument("input", help="scheduler input file")
    ap.add_argument("log", nargs="?", help="decision log (the scheduler's stdout); omit with --run")
    ap.add_argument("-e", "--expected", help="reference log to diff against")
    ap.add_argument("-o", "--out", help="html output path (default timeline.html)")
    ap.add_argument("--ascii", action="store_true", help="print to the terminal instead")
    ap.add_argument("--run", action="store_true", help="run ./a2 on the input and draw its output")
    ap.add_argument("--build", action="store_true", help="with --run, compile a2.c first")
    ap.add_argument("--timeout", type=int, default=90, help="with --run, seconds before calling it a hang")
    args = ap.parse_args(argv)
    if bool(args.log) == args.run:
        ap.error("give either a log file or --run")

    verdict = "OK"
    try:
        if args.run:
            args.log, verdict = run_first(args)
        elif not args.expected:
            sibling = args.input.rsplit(".", 1)[0] + ".expected"
            if sibling != args.log and os.path.exists(sibling):
                args.expected = sibling
        if args.out is None:
            args.out = "timeline.html"
        spec = parse_input(args.input)
        events = parse_log(args.log)
        run = replay(spec, events)
        diff = compare(events, parse_log(args.expected)) if args.expected else None
    except (LogError, OSError, IndexError, ValueError) as exc:
        print("visualiser: %s" % exc, file=sys.stderr)
        return 2

    if args.ascii:
        print(render_ascii(spec, run))
        for p in run["problems"]:
            print("  !! %s" % p)
        if diff and not diff["ok"]:
            print("  !! diverges at decision %d: expected %r, got %r"
                  % (diff["at"] + 1, diff["expected"], diff["actual"]))
        elif diff:
            print("  ok: %d decisions match, worst drift %d ms" % (diff["lines"], diff["drift"]))
    else:
        title = "%s — %s" % (args.log.rsplit("/", 1)[-1], args.input.rsplit("/", 1)[-1])
        with open(args.out, "w") as fh:
            fh.write(render_html(spec, run, diff, title))
        print("wrote %s" % args.out)

    return 1 if verdict != "OK" or run["problems"] or (diff and not diff["ok"]) else 0


if __name__ == "__main__":
    sys.exit(main())
