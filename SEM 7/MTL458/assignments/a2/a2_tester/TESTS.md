# What each test checks

47 tests in four tiers. "Rule" and "Clar." refer to the handout; "reply" to the
TA answers in `../Clarifs.md`. Setup is `n queues · S · G`, then the jobs.

Sources, in the order they win when they differ: the Piazza replies quoted in
`../Clarifs.md`, then the rest of that file, then the handout.

Every test, whatever its tier, also checks the things that hold for any run:
exact output format, termination, no leftover or unreaped child, one child on
the CPU at a time, no job running before its arrival, and a self-consistent log.
Tiers S, C and X additionally compare the decision sequence against an expected
log; tier Y does not (see the end).

## S — the handout's six samples

Graded against the handout's own expected logs, so these are the ground truth:
if one fails, nothing below it is worth reading.

| Test | Checks |
|---|---|
| `part1` | Part 1: one queue, plain round robin, fork/exec, stop/resume, finish then re-dispatch |
| `part2` | Part 2: per-level quanta and allotments, QEXPIRE vs DEMOTE |
| `part3` | Part 3: I/O mid-quantum, a partly used turn resumed, preemption on return |
| `part4_boost` | Part 4: boosts at multiples of S, re-queued in input order |
| `part4_G1` | Rule 4: counters kept across I/O, so GAMER is demoted |
| `part4_G0` | Rules 4a/4b: I/O refills the counters, so GAMER is never demoted |

## C — one clarification or rule each

| Test | Checks | Setup |
|---|---|---|
| `C01_arrive_preempt` | Clar. 4: an arrival at a strictly higher level takes the CPU at once | 3q · S=0 · G=1 · 3 jobs arriving at 0, 290, 945 |
| `C02_idle` | Reply 1 and 6: IDLE at t=0 before anything arrives, and once on each busy→idle edge — three idle stretches here, never a repeat | 2q · S=0 · G=1 · both jobs arrive late and block often |
| `C03_q0_refill` | Clar. 8: a job at Q0 is never demoted; its allotment refills | 2q · S=0 · G=1 · allot 1 at Q1, 2 at Q0 |
| `C04_n1_boost` | Rule 5 with n=1: a boost is a no-op but still prints, and re-queues in input order | 1q · S=400 · G=1 |
| `C05_boost_blocked` | Reply 2 and 6: BOOST while a job is blocked and the CPU idle, with no second IDLE | 2q · S=500 · G=1 |
| `C06_same_level_arrivals` | Clar. 4: an arrival at the same level does not preempt | 2q · S=0 · G=1 · C arrives at 100 |
| `C07_gaming_G0` | Reply 3: under G=0 the I/O refills both counters, yet demotion still waits for the whole allotment — Rule 4a alone would demote after one quantum | 3q · S=1700 · G=0 · GAMER blocks every 180 ms of its 300 ms quantum |
| `C08_gaming_G1` | The same shape of workload under G=1: the part-used turn survives the I/O, so GAMER sinks anyway — the gaming tolerance itself | 3q · S=1700 · G=1 |
| `C09_preempted_keeps_head` | Clar. 2: a preempted job returns to the **head** of its queue and keeps both counters | 2q · S=0 · G=1 · C arrives and preempts A at Q0 |
| `C10_blocked_low_across_boost` | Clar. 6: a job blocked at a low level is boosted while blocked, then joins the **tail** of the top queue | 3q · S=1000 · G=1 |
| `C11_io_end_same_level_no_preempt` | Clar. 3: a job returning from I/O at the running job's level waits its turn (no RUN line) | 2q · S=0 · G=1 |
| `C12_io_end_preempts_keeps_counters` | Clar. 2 via I/O: the job preempted by an I/O return keeps its counters | 2q · S=0 · G=1 |
| `C13_arrive_while_idle` | A job arriving while every other job is blocked: IDLE, then ARRIVE, then RUN, with no stale IDLE afterwards | 2q · S=0 · G=1 |
| `C14_G0_io_refill_stays_top` | Rule 4b: under G=0 a job blocking before its quantum ends is never demoted | 3q · S=0 · G=0 |
| `C15_io_on_quantum_end_G1` | Forum reply: I/O exactly as the turn ends prints IO_START, then QEXPIRE | 1q · S=0 · G=1 · io_every = quantum = 500 |
| `C16_io_on_quantum_end_G0` | The same under G=0: the I/O refills both counters, so **no** QEXPIRE | 1q · S=0 · G=0 · io_every = quantum = 500 |
| `C17_io_on_quantum_end_demote` | The same tie when the allotment is spent: IO_START then DEMOTE, and at Q0 a refill instead | 2q · S=0 · G=1 |
| `C18_finish_on_quantum_end` | Clar. 1: a job finishing exactly as its quantum ends prints FINISH alone, never QEXPIRE | 1q · S=0 · G=1 · cpu = quantum = 100 |

## X — timing edges, limits and same-instant ties

| Test | Checks | Setup |
|---|---|---|
| `X01_n8_ladder` | The most queues allowed, each with its own quantum and allotment, so a job demotes down all eight levels in turn | 8q · S=0 · G=1 · quanta 50…2000, allotments 1…3 |
| `X02_16_jobs` | The most jobs allowed, all arriving together in one queue: 166 decisions of pure round robin | 1q · S=0 · G=1 · 16 jobs |
| `X03_late_start_boosts` | Nothing arrives for 9.5 s: IDLE at 0, boosts printed into an empty system (reply 2) | 2q · S=3000 · G=1 · arrivals at 9500, 10000 |
| `X04_min_quantum` | The smallest quantum allowed, so switches come thick and fast | 3q · S=0 · G=1 · every quantum 50 |
| `X05_io_never_fires` | One job with `io_every` equal to its cpu and one with more: `workload` performs the I/O only *before* its last unit, so neither ever blocks | 2q · S=0 · G=1 |
| `X06_long_names` | 15-character names and a 1-character name, printed exactly | 2q · S=0 · G=1 |
| `X07_boost_starves_B` | Boosts faster than the quantum (S=200 < 300): every boost re-queues in input order, so A is picked every time and B does not run until A is done | 3q · S=200 · G=1 |
| `X08_arrive_on_boost` | **Clar. 7:** an arrival at the same instant as a boost prints ARRIVE, BOOST, then one RUN | 2q · S=500 · G=1 · B arrives at 500 |
| `X09_long_io_idle_boosts` | Boosts landing during a long idle stretch, with no second IDLE | 2q · S=1000 · G=1 · I/O of 3.2 s and 4.1 s |
| `X10_everyone_q0` | Three jobs all sink to Q0 and share it round robin, refilling their allotment there instead of demoting | 2q · S=0 · G=0 |
| `X11_arrival_tie_input_order` | Clar. 7: two jobs arriving at the same instant go in input order, not alphabetical | 2q · S=0 · G=1 · ZED and ALF both arrive at 420 |
| `X12_three_arrivals_on_boost` | Clar. 7 again: three ARRIVEs, then BOOST, then a single RUN | 2q · S=500 · G=1 · B, C, D all arrive at 1000 |
| `X13_arrive_on_boost_while_idle` | The same tie while the CPU is idle | 2q · S=400 · G=1 · B arrives at 800 |
| `X14_single_job_long` | One job with no competition: it walks down every level, refills at Q0, and each boost lifts it back to the top | 3q · S=2500 · G=1 · one 5.9 s job |
| `X15_max_quantum_allot` | The largest quantum and allotment allowed | 2q · S=0 · G=1 · quantum 2000, allot 8 |
| `X16_long_io_outlives_others` | A 2.5 s I/O outlives every other job: the CPU idles, then the job returns | 2q · S=0 · G=1 |
| `X17_many_blocked_across_boost` | Two jobs blocked when the boost lands: each joins the top queue at its own IO_END, in completion order, behind whoever is already there | 3q · S=1000 · G=1 |

## Y — stress

Deliberately dense: events land on top of each other, so no single order is
right on a real machine. These are **not** compared against an expected log.
They check that the scheduler survives: no hang, no crash, no leftover process,
one child on the CPU at a time, and a log that stays self-consistent.

| Test | Checks | Setup |
|---|---|---|
| `Y01_io_storm` | Constant pipe traffic: 8 jobs each blocking every 20–41 ms, so messages arrive in bursts and often mid-switch | 3q · S=700 · G=1 |
| `Y02_16_mixed_long` | A 30 s run mixing CPU-only and I/O jobs arriving throughout | 4q · S=1500 · G=0 · 16 jobs |
| `Y03_pipe_burst_max_boost` | Many short messages at once, with the most frequent boost allowed | 3q · S=200 · G=0 · 16 jobs |
| `Y04_io_on_boost_edges` | Every job's `io_every` is a multiple of S, so IO_START keeps landing on a boost — the ordering that crashed the scheduler before `q_remove` | 3q · S=200 · G=1 · 10 jobs |
| `Y05_finish_on_quantum_edges` | 12 jobs whose cpu sits 1 ms either side of a quantum boundary, so finishes and expiries keep colliding (Clar. 1) | 2q · S=0 · G=1 |
| `Y06_n8_m16_mixed_G0` | Everything at once: 8 queues, 16 jobs, G=0, staggered arrivals, 18 s | 8q · S=700 · G=0 |

## U — your own tests

Any other `cases/*.txt` lands here. Write the input, optionally an `.expected`
next to it, and run `run_tests.py -m <name>`. See README.md.
