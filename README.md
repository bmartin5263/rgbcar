# rgbcar
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/bdon/library/rgbcar.svg)](https://registry.platformio.org/libraries/bdon/rgbcar)

Vehicle-based extension to rgblib adding OBD-II support

## Instrumentation

If an SD card is detected, the Vehicle process will log vehicle data at a configurable interval to a log file on the SD card

The format is in binary, use `vlog-to-csv.py` to convert it to CSV for analysis

See Also
- `class VehicleLogger`
- `struct VehicleData`