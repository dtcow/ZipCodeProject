# ZIP Code Index Tests

Compile the application and run `python3 test_step_ii.py` from the project
folder. See [DEMONSTRATION.txt](DEMONSTRATION.txt) for the captured output.

| Case | Expected and observed |
| --- | --- |
| `us_postal_codes.csv` | Builds 40,933 index entries. Three valid ZIPs match all six expected fields. `00000` is reported missing. |
| `us_postal_codes_by_place.csv` | Same results with a different source row order. |
| Both inputs with columns reordered | Same output for both files. The test moves ZIP to the fourth column. |
| All four generated index files | Independent Python CSV decoding verifies each of the 40,933 offsets and compares all fields with the corresponding original input. |
| State-extremes regression | The extremes table is byte-identical across all four CSVs and all four corresponding length-indicated files. The default table also matched the original C++ program's table. |

The demonstration uses the two CSVs included in the starter ZIP. Its
second input is the supplied place-ordered copy.
