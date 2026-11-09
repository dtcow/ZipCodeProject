# ZIP Code Index Design

## Build and search

The build command reads one CSV record at a time through `Buffer`. It writes
that record to the binary-mode data stream and stores the normalized five-digit
ZIP with the byte offset immediately before the record's length prefix. After
the scan, it sorts the entries by ZIP, rejects duplicate primary keys, updates
the header's fixed-width record count, and writes the sorted index to disk.

The search command opens the data file, reads its header, and loads the
companion index into a vector. Each `-Z` option uses binary search on that vector.
A match triggers a seek directly to the byte offset and reads the indicated
record. One `ZipCodeRecord` is reused for all lookups. Temporary parsing storage
is limited to the current record. The state-extremes map is only created in the
separate extremes mode.

## File format

| Header item | Version 1 representation |
| --- | --- |
| Structure and version | `ZIP-LENGTH 1` |
| Total header byte count | 10 zero-padded ASCII digits after the signature and version |
| Data record length width | 10 ASCII bytes |
| Length inclusion | Payload length excludes the 10-byte prefix |
| Companion index | Quoted filename relative to the data file's directory |
| Record count | 20 zero-padded ASCII digits |
| Schema | Six fields with names and text or numeric types |
| Primary key ordinal | 1, the ZIP code field |

Records store a length prefix followed immediately by a CSV payload in the
canonical order ZIP, place, state, county, latitude, longitude. Quoted text
uses doubled quotes. The binary-mode streams keep offsets and byte counts
consistent across platforms. Each index line stores `ZIP,offset`. The index
loader checks key format, sort order, minimum offsets, and record count. Each
successful search also checks that the record at the offset has the requested key.

## Source changes

| File | Change |
| --- | --- |
| `Buffer.h`, `Buffer.cpp` | Reuse the existing CSV tokenizer and field validation. Map columns by header name, retain the county, and add length-prefixed record I/O and seeking. |
| `HeaderBuffer.h`, `HeaderBuffer.cpp` | New header reader and writer with version and size checks. |
| `IndexEntry.h` | Keep the existing ZIP/offset struct and use `std::streamoff` for the offset. |
| `Main.cpp` | Replace placeholder offsets with actual file positions and add build and lookup modes. Move the original state-extremes loop and table into a function so both data formats can use it. |
