"""Reproduce the four Part II demonstrations and independently verify the disk files."""
import csv
import io
from pathlib import Path
import subprocess
import tempfile


PROJECT = Path(__file__).resolve().parent
PROGRAM = PROJECT / "myprogram"
SOURCES = ("us_postal_codes.csv", "us_postal_codes_by_place.csv")
FLAGS = ("-Z00501", "-Z56301", "-Z90210", "-Z00000")
EXPECTED = """ZIP Code: 00501 | Place Name: "Holtsville" | State: NY | County: "Suffolk" | Latitude: 40.8154 | Longitude: -73.0451
ZIP Code: 56301 | Place Name: "Saint Cloud" | State: MN | County: "Stearns" | Latitude: 45.541 | Longitude: -94.1819
ZIP Code: 90210 | Place Name: "Beverly Hills" | State: CA | County: "Los Angeles" | Latitude: 34.0901 | Longitude: -118.4065
ZIP Code 00000 was not found.
"""


def run(*args):
    """Run one program invocation and return its standard output."""
    return subprocess.check_output([PROGRAM, *args], text=True)


def reorder(source, destination):
    """Move ZIP to the fourth column and change the other field positions."""
    order = (5, 2, 3, 0, 4, 1)
    with source.open(encoding="utf-8-sig", newline="") as original:
        with destination.open("w", encoding="utf-8", newline="") as reordered:
            writer = csv.writer(reordered)
            for row in csv.reader(original):
                if row:
                    writer.writerow([row[column] for column in order])


def verify_offsets(source, data):
    """Compare every indexed record with its source fields and verify each byte offset."""
    expected = {}
    with source.open(encoding="utf-8-sig", newline="") as original:
        rows = csv.reader(original)
        next(rows)
        for zip_code, place, state, county, lat, lon in rows:
            expected[zip_code.zfill(5)] = (place, state, county, float(lat), float(lon))

    with data.open("rb") as records, Path(str(data) + ".idx").open() as index:
        signature, version, header_size = records.readline().split()
        assert (signature, version) == (b"ZIP-LENGTH", b"1")
        assert records.read(int(header_size) - records.tell()).endswith(b"primary_key_field=1\n")
        for expected_zip, entry in zip(sorted(expected), index, strict=True):
            key, offset = entry.rstrip().split(",")
            assert key == expected_zip
            records.seek(int(offset))
            size = int(records.read(10))
            payload = io.StringIO(records.read(size).decode("utf-8"), newline="")
            ((zip_code, place, state, county, lat, lon),) = csv.reader(payload)
            assert zip_code == expected_zip
            assert (place, state, county, float(lat), float(lon)) == expected[key]
    return len(expected)


def main():
    """Compare both supplied inputs and their reordered versions in fresh files."""
    baseline = run(PROJECT / SOURCES[0])
    with tempfile.TemporaryDirectory() as folder:
        work = Path(folder)
        for name in SOURCES:
            original = PROJECT / name
            reordered = work / f"{original.stem}_reordered.csv"
            reorder(original, reordered)
            for source in (original, reordered):
                data = work / f"{source.stem}.zdat"
                print(f"$ ./myprogram --build {source.name} {data.name}")
                build = run("--build", source, data)
                assert build == f"Indexed 40933 ZIP codes in {data}\n"
                print(build.replace(str(data), data.name), end="")
                print(f"$ ./myprogram {data.name} {' '.join(FLAGS)}")
                output = run(data, *FLAGS)
                assert output == EXPECTED
                print(output, end="")
                assert run(source) == baseline
                assert run("--extremes", data) == baseline
                count = verify_offsets(original, data)
                print(f"Verified {count} index offsets, all record fields, and both extremes modes.\n")


if __name__ == "__main__":
    main()
