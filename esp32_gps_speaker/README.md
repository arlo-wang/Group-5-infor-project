
## pins used so far: 



## NMEA 0183 Protocol

The GPS module communicates using the NMEA 0183 protocol, which consists of standardized sentences that contain various types of navigation data.

### Basic NMEA Sentence Structure

```
$AABBB,data1,data2,data3,...*XX
```

Where:
- **$** - Start of sentence marker
- **AA** - Talker ID (GP=GPS, GL=GLONASS, GA=Galileo, BD=BeiDou, GN=Multiple systems)
- **BBB** - Sentence type identifier
- **data1,data2,...** - Comma-separated data fields
- **\*XX** - Checksum (two hexadecimal digits)
- Each sentence ends with carriage return and line feed characters (\r\n)

### Common NMEA Sentence Types

#### 1. GGA - Global Positioning System Fix Data

```
$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
```

Fields:
1. Time (UTC): 123519 = 12:35:19
2. Latitude: 4807.038 = 48 degrees 07.038 minutes
3. N/S Indicator: N = North
4. Longitude: 01131.000 = 11 degrees 31.000 minutes
5. E/W Indicator: E = East
6. Fix Quality:
   - 0 = Invalid
   - 1 = GPS fix
   - 2 = DGPS fix
7. Number of satellites: 08
8. Horizontal dilution of precision: 0.9
9. Altitude: 545.4 meters above mean sea level
10. Altitude units: M = meters
11. Geoidal separation: 46.9 meters
12. Geoidal separation units: M = meters
13. Age of differential GPS data (blank)
14. Differential reference station ID (blank)
15. Checksum: *47

#### 2. RMC - Recommended Minimum Navigation Information

```
$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
```

Fields:
1. Time (UTC): 123519 = 12:35:19
2. Status: A = Valid, V = Invalid
3. Latitude: 4807.038 = 48 degrees 07.038 minutes
4. N/S Indicator: N = North
5. Longitude: 01131.000 = 11 degrees 31.000 minutes
6. E/W Indicator: E = East
7. Speed over ground: 022.4 knots
8. Course over ground: 084.4 degrees
9. Date: 230394 = March 23, 1994
10. Magnetic variation: 003.1 degrees
11. Magnetic variation direction: W = West
12. Checksum: *6A

#### 3. GSV - Satellites in View

```
$GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
```

Fields:
1. Total number of GSV messages: 3
2. Message number: 1 (this is the first message)
3. Total satellites in view: 11
4. Satellite information (repeated for up to 4 satellites per message):
   - Satellite PRN number: 03
   - Elevation: 03 degrees (0-90)
   - Azimuth: 111 degrees (0-359)
   - Signal-to-noise ratio: 00 dB (00-99, 00 means not tracking)
5. Checksum: *74

#### 4. GSA - Satellite Status

```
$GPGSA,A,3,04,05,09,12,,,,,,,,,2.5,1.3,2.1*39
```

Fields:
1. Mode: A = Automatic, M = Manual
2. Fix type:
   - 1 = No fix
   - 2 = 2D fix
   - 3 = 3D fix
3. PRNs of satellites used for fix (up to 12)
4. Position dilution of precision (PDOP): 2.5
5. Horizontal dilution of precision (HDOP): 1.3
6. Vertical dilution of precision (VDOP): 2.1
7. Checksum: *39

### Multi-GNSS Systems

Modern GPS modules often support multiple satellite navigation systems:
- **$GP** - GPS (USA)
- **$GL** - GLONASS (Russia)
- **$GA** - Galileo (EU)
- **$BD** - BeiDou (China)
- **$GN** - Multiple systems combined

