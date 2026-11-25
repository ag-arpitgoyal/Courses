# Per-lecture review: MTL730 post-midsem LaTeX notes

10 reviewers run IN PARALLEL, one per lecture (13-22). WORK QUICKLY: one focused pass, fix real
problems, don't polish correct text, don't over-investigate. Finish and report.

## Files
- Main: `/home/arpit_goyal/Desktop/SEM-7/MTL730/Lecture_Notes/Post_Mid_Sem.tex` (preamble + \input of parts)
- Body: `/home/arpit_goyal/Desktop/SEM-7/MTL730/Lecture_Notes/post_mid_parts/partN.tex`
  Each scan page starts at a `% --- page NN ---` marker; each lecture at `\lectureheading{Lecture NN ...}`.
- Handwritten scans: `/tmp/mtl730/hi/p-NN.jpg` (NN=01..73). For hard regions, zoom:
  `convert /tmp/mtl730/hi/p-NN.jpg -crop WxH+X+Y <yourdir>/z.jpg`, or re-render from
  `/home/arpit_goyal/Desktop/SEM-7/MTL730/MTL730_Post_Mid_Sem.pdf` with `pdftoppm -r 200 -f N -l N -jpeg`.
- Style reference (do not edit): `Lectures_1_9-12.tex` in the same folder.

## Check your lecture for
1. Completeness vs the handwritten pages: every definition, theorem, proof step, example, side remark.
2. Math: every formula and number matches the scan or is a correct fix; verify computations with python3.
   `% FIX` comments mark earlier deliberate corrections — confirm they are right; `% UNCLEAR` — resolve if you can.
3. Formatting in the compiled PDF: render your lecture's pages (`pdftoppm -r 80 -png -f A -l B`) and look:
   broken/ugly math, overflowing lines, wrong environment, stray artifacts, figures correct vs scan
   (labels, points on curves, arrows), labels not overlapping.

## Editing rules (others edit the same partN.tex files concurrently)
- Only edit inside YOUR lecture's line range (from your \lectureheading to the next one).
- Small targeted StrReplace edits only; re-read the region right before each edit. Never rewrite a file.
- Don't renumber/rename lectures or sections; don't touch the preamble.
- Compile in your own dir (mkdir first, run twice):
  `cd /home/arpit_goyal/Desktop/SEM-7/MTL730/Lecture_Notes && pdflatex -interaction=nonstopmode -halt-on-error -output-directory=/tmp/mtl730/lec<NN> Post_Mid_Sem.tex`
  If it fails in someone else's range, wait ~30s and retry. Never write build files into Lecture_Notes,
  never overwrite Post_Mid_Sem.pdf.

## Final reply (short)
- Fixes made (one line each, with scan page).
- Issues found but not fixed (one line each).
- Compile status.
