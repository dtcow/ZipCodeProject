# Reviewed project copy

## What was corrected

The original uploaded source printed place names rather than ZIP codes, used an unordered table without a header, compared southern latitude against longitude, reversed the assignment's longitude comparisons, and updated westMostLon when updating the eastern candidate. It also lacked Doxygen tags and header guards. The uploaded CSV has a multiline header, so hard-coded skipping of three physical lines only worked for that exact header format.

This copy separates sequential CSV input from state aggregation, prints ZIP codes in the required column order, sorts IDs, preserves leading zeros, handles quoted CSV fields, validates input, and breaks coordinate ties by smallest ZIP. The supplied data is preserved, with normalized one-line column headers and two different row sort orders.

Your uploaded myprogram is a compiled Linux x86-64 executable. Windows cannot run it directly as a native Windows application. The source files are the authoritative deliverables; compile them for your own environment.

## Build and run (Linux, WSL, or a university Linux terminal)

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic Main.cpp Buffer.cpp -o myprogram
./myprogram us_postal_codes.csv
./myprogram us_postal_codes_by_place.csv
```

## Capture fresh terminal transcripts after group review

The package contains actual transcripts captured in the review environment. If your instructor expects transcripts from your own machine, regenerate them after your group's final edits:

```sh
script run_zip.typescript
./myprogram us_postal_codes.csv
exit
script run_place.typescript
./myprogram us_postal_codes_by_place.csv
exit
./myprogram us_postal_codes.csv > output_zip.txt
./myprogram us_postal_codes_by_place.csv > output_place.txt
diff output_zip.txt output_place.txt
```

No output from diff means the tables match.

## Generate the required Doxygen PDF

The uploaded chat screenshot shows HTML documentation. HTML is not the required PDF. Doxygen is not installed in the review environment, so this package does NOT contain a Doxygen-generated PDF.

On a computer with Doxygen, a LaTeX distribution, and make installed, run from this project folder:

```sh
doxygen Doxyfile
cd docs/latex
make
```

The expected PDF is docs/latex/refman.pdf. On Windows, Doxygen's generated LaTeX folder generally contains make.bat; run it with a working LaTeX installation. Keep the Doxyfile filename exactly as supplied in this package.

## Before submission

1. Have the group review the corrected code and confirm any lecture-specific buffer requirements.
2. Resolve the assignment's east/west wording with the instructor: this code follows east = least longitude literally, although signed U.S. longitudes normally imply the opposite geographic direction.
3. Confirm whether all 57 postal IDs should be included.
4. Add group names to the design draft and confirm the preliminary design was submitted at least one day before source submission. This newly prepared draft does not establish that timing.
5. Generate and inspect the Doxygen PDF.
6. Submit the final source files, CSV, design document, Doxygen PDF, and both transcripts, following the course's packaging instructions.
