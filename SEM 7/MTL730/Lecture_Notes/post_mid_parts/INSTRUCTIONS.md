# Task: transcribe handwritten lecture scans into LaTeX (MTL730, Cryptography course)

Source: scanned handwritten class notes, one JPEG per page at `/tmp/mtl730/hi/p-NN.jpg` (NN = 01..73).
Use the Read tool on each image path to view it. If a page is hard to read, crop/zoom with
`convert /tmp/mtl730/hi/p-NN.jpg -crop WxH+X+Y /tmp/mtl730/zoom_NN.jpg` (or re-render from
`/home/arpit_goyal/Desktop/SEM-7/MTL730/MTL730_Post_Mid_Sem.pdf` with `pdftoppm -r 200 -f N -l N -jpeg`).

These notes continue an existing LaTeX document:
`/home/arpit_goyal/Desktop/SEM-7/MTL730/Lecture_Notes/Lectures_1_9-12.tex`
Read its preamble (lines 1-100) and one representative section (e.g. lines 1360-1480) to copy its style.
Do NOT edit that file.

## Output
Write ONLY a LaTeX body fragment (no preamble, no \begin{document}) to the output file given in your prompt.
It will be \input into a document that already has this preamble:
- packages: amsmath, amssymb, amsthm, mathtools, enumitem, graphicx, array, float, tikz, hyperref
- environments: theorem, lemma, proposition, corollary, definition, example, claim (numbered), remark (unnumbered), proof
- macros: \Z \N \R \Fp \units{n} (=(Z/nZ)^x), \gcdop, \ord, \ind, \lectureheading{...}
If you need another macro/package, do not define it; write it out in full instead (e.g. \operatorname{lcm}).

## Rules
1. Be COMPLETE: every definition, theorem, proof, worked example, numeric computation, table, and
   side remark on the pages must appear. The user wants to miss nothing. Do not summarise away steps.
2. Clean up: write in full sentences like the existing notes, fix obvious arithmetic/notation slips
   (and add `% FIX: ...` comment saying what you changed), fill in tiny skipped steps. Keep all examples.
3. Structure: `\section{...}` for each new topic, `\subsection{...}` for sub-topics; use the theorem-like
   environments. When a page shows a new lecture date / lecture number, insert
   `\lectureheading{Lecture ?? (DATE)}` (write the date as written; the number gets filled in later).
   If your first page continues a topic from the previous page, start with `% CONTINUES: <topic>` and
   just continue the content (no new \section unless a new topic starts).
4. Diagrams/figures are IMPORTANT and must be accurate. Redraw EVERY diagram, graph, curve plot,
   protocol flow (Alice/Bob arrows), circuit/round structure, table, and matrix in TikZ (or tabular/array).
   Match the original: same labels, arrows, points, values, axis shapes. For curves (e.g. elliptic curves)
   use TikZ plotting with correct equations, mark the points shown and lines through them.
   Wrap in `\begin{figure}[H]\centering ... \caption{...}\end{figure}`. After compiling, convert the
   test PDF pages containing your figures to PNG (`pdftoppm -r 80 -png`) and LOOK at them with Read
   to check they are correct, not overlapping, and readable. Fix until they are good.
5. Illegible content: make your best guess and mark it `% UNCLEAR p.NN: ...`.
6. Put a comment `% --- page NN ---` at the point where each source page starts.
7. The file must compile. Verify by creating a temp wrapper in /tmp that copies the preamble
   (lines 1-99 of Lectures_1_9-12.tex, i.e. up to and including \begin{document}), \input's your
   fragment, and \end{document}; run `pdflatex -interaction=nonstopmode -halt-on-error` in /tmp. Fix errors.
   Never write build files into the Lecture_Notes folder.

## Final reply (keep it short)
- Per page: one line `pNN: <topics>; <date/lecture marker if any>`.
- List of UNCLEAR / FIX items.
- Whether the compile test passed.
