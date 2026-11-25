# Review task: MTL730 post-midsem LaTeX notes

Several reviewers run IN PARALLEL on the same files, each with a different focus (given in your prompt).

## Files
- Main: `/home/arpit_goyal/Desktop/SEM-7/MTL730/Lecture_Notes/Post_Mid_Sem.tex` (preamble + `\input` of parts)
- Body: `/home/arpit_goyal/Desktop/SEM-7/MTL730/Lecture_Notes/post_mid_parts/part1.tex` .. `part6.tex`
  (scan pages: part1=1-12, part2=13-24, part3=25-37, part4=38-49, part5=50-61, part6=62-73;
  each source page is marked by `% --- page NN ---`)
- Compiled PDF (65 pages): `/home/arpit_goyal/Desktop/SEM-7/MTL730/Lecture_Notes/Post_Mid_Sem.pdf`,
  pre-rendered pages: `/tmp/mtl730/out/o-NN.png` (view with Read)
- Original handwritten scans: `/tmp/mtl730/hi/p-NN.jpg` (NN=01..73). Zoom with
  `convert ... -crop WxH+X+Y` into your own temp dir if needed.
- Previous notes the style must follow: `/home/arpit_goyal/Desktop/SEM-7/MTL730/Lecture_Notes/Lectures_1_9-12.tex` (do not edit)
- Original transcription rules: `post_mid_parts/INSTRUCTIONS.md`

## Editing rules (other reviewers edit the same files concurrently)
- Only edit within YOUR focus area. Report anything else instead of fixing it.
- Use small, targeted StrReplace edits only. Never rewrite/overwrite a whole file. Re-read the region right
  before editing.
- Do not edit Post_Mid_Sem.tex preamble unless your focus is formatting, and then only minimally.
- Compile check in your OWN temp dir:
  `cd /home/arpit_goyal/Desktop/SEM-7/MTL730/Lecture_Notes && pdflatex -interaction=nonstopmode -halt-on-error -output-directory=/tmp/mtl730/rev_<yourletter> Post_Mid_Sem.tex`
  (mkdir it first; run twice). If it fails because of someone else's in-progress edit, wait ~30s and retry.
  Never write build files into Lecture_Notes and never overwrite Post_Mid_Sem.pdf.
- Work fast: prioritise real errors over nitpicks. Don't change correct content for taste.

## Final reply (short)
- List of fixes made (file:approx line, one line each).
- List of issues found but NOT fixed (outside your focus or uncertain), one line each.
- Compile status.
