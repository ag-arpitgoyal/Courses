# Extra MLFQ cases

One line each: what the test checks, and which rule or TA clarification it targets. Expected logs are the tester oracle's exact decision sequence. No oracle/hand disagreement remains on these finals.

E01_g0_preempt_keeps_turn — G=0: B arrives at the top and preempts A mid-Q0-turn; A resumes the leftover 650 ms (QEXPIRE at 1400, not 1550). TA 4: preemption never refills, even under G=0.
E02_g1_preempt_keeps_turn — Same jobs as E01 with G=1; the log is identical. TA 3–4: G only changes I/O behaviour, not preemption.
E03_boost_midturn_q0 — A is 200 ms into an 800 ms Q0 turn when S fires; BOOST refills a full top quantum (next DEMOTE 200 ms later, not 600). Rule 5 / Clar. 5: boost refills both counters.
E04_io_spans_two_boosts — A's I/O covers BOOST at 400 and 800 while the CPU is idle; no second IDLE; A then joins the already-boosted top queue. TA 2 and 6; Clar. 6.
E05_q0_rr_io_g1 — Two jobs at Q0, A blocks mid-turn; on return A is behind B and resumes the leftover 300 ms (QEXPIRE A at 1400). Rule 4 / Clar. 3: I/O does not refill when G=1; return joins the tail.
E06_q0_rr_io_g0 — Same shape as E05 with G=0: A's I/O refills a full Q0 turn, so A finishes at 1450 with no QEXPIRE. Rule 4b.
E07_idle_boosts_then_arrive — IDLE at 0, BOOST into an empty system at 500 and 1000, then A arrives at 1200 (not on a boost). TA 1–2, 6; Clar. 10: no BOOST after the last FINISH.
E08_n1_io_and_boost — n=1 with I/O: B finishes, IDLE, BOOST while A is blocked, then IO_END / RUN. Rule 5 with n=1; TA 2 and 6. Complements C04 (n=1 boost, no I/O).
E09_allot1_g1_io — Allotment 1 on every level, G=1: leftover of the top quantum then DEMOTE; later I/O exactly at the Q1 quantum end prints IO_START then DEMOTE. Rule 4; TA 7 (G=1).
E10_allot1_g0_io — Same jobs as E09 with G=0: each I/O refills Q2, so A never demotes and the last slice finishes without a fifth I/O. Rule 4b; TA 3; I/O-at-cpu-end does not fire.
E11_n8_boost_rescue — Eight queues, allotment 1: both jobs are mid-ladder (B mid-Q4) when S=850 lifts them to Q7 in input-file order; they never reach Q0. Rule 5 / Clar. 6. Complements X01 (n=8, no boost).
E12_preempt_head_vs_io_tail — C preempts B (B goes to the head of Q0) while A is blocked; A later joins the tail. After C finishes, B runs, not A. Clar. 2 vs Clar. 3.
E13_three_arrive_preempt — ZED, ALF, BOB arrive together while M is at Q0; ARRIVE order is input-file order (not alphabetical), then they preempt. Clar. 4 and 7. Complements X11 (two arrivals) and X12 (arrivals on a boost).
E14_g0_io_qend_no_demote — G=0, I/O exactly as the Q1 quantum ends with allotment 1: IO_START only, no DEMOTE, A stays at Q1. Rule 4b; TA 7 (G=0). Complements C17 (same tie under G=1, which does demote).
E15_g0_io_then_preempt — G=0 I/O at Q0 refills a full 500 ms turn; B later preempts 120 ms into that turn; A resumes 380 ms (QEXPIRE at 1680, not 1800). Rule 4b then TA 4.
E16_boosted_blocked_behind_new — A is boosted while blocked; B then C arrive at the top; A's IO_END joins the tail, so C runs after B, not A. Clar. 6 and 3.
E17_all_blocked_boost — Both jobs block, IDLE, BOOST at 600 with no second IDLE; later B blocks again and BOOST at 1200 while A is already done. TA 2 and 6.
E18_n1_g0_io_refill — n=1, G=0: I/O 250 ms into a 400 ms turn refills, so after IO_END A runs 200 ms and finishes with no QEXPIRE (G=1 would QEXPIRE after the leftover 150). Rule 4b.
E19_arrive_after_boost_same_level — Boost lifts A to Q1; B arrives 100 ms later at the same level and does not preempt; A runs out the top quantum first. Clar. 4 after a Rule 5 boost.
E20_two_io_end_tail_order — ALF's I/O ends first, so ALF is dispatched before ZED (who is earlier in the file); ZED then waits behind CPU. Clar. 3: IO_END joins the tail, in completion order.
E21_demote_then_arrive — A's Q1 quantum ends at B's arrival: DEMOTE, ARRIVE, RUN B (B at top, A now at Q0). Gap A; Clar. 7. Oracle race warning is the defined 0 ms tie.
E22_demote_then_boost — Quantum end exactly on S=200: DEMOTE then BOOST then RUN, three times. Gap B; Clar. 7. Defined 0 ms ties.
E23_io_end_then_boost — A's I/O ends at t=400 = S: IO_END, BOOST, RUN A. Gap C; Clar. 7. Defined 0 ms tie.
E24_io_end_then_arrive — A's I/O ends at B's arrival: IO_END, ARRIVE, RUN A (A already at the top head). Gap C; Clar. 7. Defined 0 ms tie.
E25_io_end_then_qexpire — A's IO_END and B's QEXPIRE at t=350: IO_END first, then QEXPIRE B to tail, RUN A. Gap D; Clar. 7. Defined 0 ms tie.
E26_g0_full_quantum_then_io — G=0: two I/O-less full quanta at Q2 (QEXPIRE then DEMOTE) prove allotment still applies; a later I/O at Q1 does refill. Gap E; TA 3.
E27_g1_io_then_higher_preempt — G=1 leftover after I/O at Q0, then B arrives at Q1 and preempts; A later uses the leftover (QEXPIRE at 1080). Gap F; Clar. 2–4.
E28_q0_allot_exhaust_rr — Two jobs at Q0: A exhausts a quantum, QEXPIRE, goes to tail, B runs; later A exhausts allotment at Q0 and refills (Clar. 8) still to tail. Gap G.
E29_n1_late_arrive_rr — n=1: B arrives at 200 while A is running (same level, no preempt); A's QEXPIRE sends A to tail, then B runs. Gap H; Clar. 4 and 8.
E30_boost_mixed_input_order — At the boost: BOB is running at Q1, ALF is ready at Q0, ZED is blocked; top rebuild is ALF then BOB (input order), not the runner first. Gap I; Clar. 6.
E31_min_s_many_boosts — S=200 (minimum) with BOOST at 200, 400, 600; last FINISH at 660 so no further BOOST. Gap J; TA 2.
E32_io_suppressed_at_finish — Next I/O would be at exactly remaining-CPU=0 (t=580) but that is not a quantum end: FINISH only, never IO_START. Gap K.
E33_late_pair_same_arrival — IDLE at 0, then ZED and ALF arrive together at 300 (not t=0): ARRIVE in input order, RUN ZED. Gap L; Clar. 7.
E34_g1_multilevel_io — 3 queues, allotment 2, G=1: I/O leftover at Q2 then DEMOTE; further I/Os at Q1 with leftover QEXPIRE/DEMOTE. Gap M.
E35_g0_multilevel_io — Same jobs as E34 with G=0: each I/O refills Q2 so A never leaves the top. Gap M; Rule 4b.
E36_io_end_lower_no_preempt — A returns from I/O at Q0 while B is running at Q1: IO_END, no RUN A until B finishes. Gap N.
E37_io_end_arrive_after_boost — A is boosted while blocked; IO_END and B's ARRIVE at t=800: IO_END, ARRIVE, RUN A. Gap O; Clar. 7. Defined 0 ms tie.
E38_g0_refill_then_demote — G=0 I/O at Q1 refills; A then uses a full 400 ms quantum and DEMOTEs (allotment spent on post-refill turns). Gap P; TA 3.

Oracle vs hand: none of the final logs disagree. A few first-pass hand traces were wrong (missed a later I/O, or forgot that a boost while blocked refills the leftover); the oracle matched the spec and those tests were redesigned so events stay ≥25 ms apart. E21–E25 and E37 are deliberate Clar. 7 same-instant ties; the oracle prints a race warning (child-timed vs absolute/other child) but still emits one spec-ordered log, which is what the .expected files use.
