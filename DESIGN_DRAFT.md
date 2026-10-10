# Zip Code Group Project 1.0 — Design draft

Group members: add your group's names before submission.

This is a draft prepared during review, not evidence of an earlier preliminary submission. The assignment requires the preliminary design to be submitted at least one day before the C++ source files. Confirm that the group has already met that requirement.

## Objective and input

Read the provided ZIP CSV sequentially and print four extreme ZIP codes for each state/postal ID in alphabetical order. The input column order is ZIP Code, Place Name, State, County, Lat, Long. ZIP codes are strings padded to five digits.

## Classes and responsibilities

- ZipCodeRecord stores one ZIP, place, state ID, latitude, and longitude.
- Buffer owns an input file stream and parses one logical CSV record at a time. It consumes one logical header, even when the quoted header spans physical lines. Its next method gathers a ZipCodeRecord and validates its fields.
- State stores four candidate records and updates them using coordinate comparisons. Coordinate ties choose the smallest ZIP code.
- Main creates the buffer, gathers each record, updates an alphabetical std::map of State summaries, and prints a labeled table.

## Algorithm

1. Open and validate the CSV header.
2. Read the next record until end of file.
3. Initialize the state's four candidates on its first occurrence.
4. Otherwise compare longitude and latitude and update the appropriate candidates.
5. Iterate through the ordered map and print State, Easternmost, Westernmost, Northernmost, Southernmost.

The screenshot explicitly defines easternmost as least longitude. This implementation follows that wording: east is minimum longitude, west is maximum longitude, north is maximum latitude, and south is minimum latitude. With conventional signed negative U.S. longitudes, geographic east/west normally use the opposite comparisons. Ask the instructor to resolve that discrepancy before changing the implementation.

Every two-character postal ID present in the input is retained. The supplied dataset has 57 IDs, including territories and military postal IDs. Confirm whether the instructor wants those included or wants only U.S. states and DC; no unrequested filtering is applied.

Processing time is O(n log s), where n is the record count and s is the number of IDs. Storage is O(s + L), where L is the longest logical CSV record. No complete input table is stored in memory by the C++ program.

## Verification plan

Run the same program on identical data sorted by ZIP and by Place Name. Compare output files, capture two real terminal transcripts, check the extremes independently, and test ties, quoted commas, invalid coordinates, and missing input.

## Documentation and deliverables

Document the classes and methods with Doxygen tags, run Doxygen, build its LaTeX output into a PDF, and submit that PDF together with the source, CSV, preliminary design, and two terminal transcripts.

If class lectures require a fixed-size char array, explicit field widths, or a specific buffer interface, adapt Buffer to those requirements before submission; those details are not specified in the uploaded assignment screenshot.
