Run the Part II demonstration. Each flag takes a five-digit ZIP with no
   space after `-Z`. Enter leading zeros when needed.

   ```bash
   ./myprogram zip.zdat -Z00501 -Z56301 -Z90210 -Z00000
   ./myprogram place.zdat -Z00501 -Z56301 -Z90210 -Z00000
   ```

   Both runs should print:

   ```text
   ZIP Code: 00501 | Place Name: "Holtsville" | State: NY | County: "Suffolk" | Latitude: 40.8154 | Longitude: -73.0451
   ZIP Code: 56301 | Place Name: "Saint Cloud" | State: MN | County: "Stearns" | Latitude: 45.541 | Longitude: -94.1819
   ZIP Code: 90210 | Place Name: "Beverly Hills" | State: CA | County: "Los Angeles" | Latitude: 34.0901 | Longitude: -118.4065
   ZIP Code 00000 was not found.
   ```

   ```bash THIS WAS MADE WITH AI, Read AI Statment document for details.
   python3 test_step_ii.py
   ```

