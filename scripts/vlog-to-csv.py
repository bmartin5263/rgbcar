#!/usr/bin/env python3
"""Convert binary vehicle log files (vlog_*.bin) to CSV."""

import struct
import sys
import os

# Must match VehicleLogEntry layout:
#   u64  timestamp          (microseconds since boot)
#   i32  lastUpdateResult   (0=NONE, 1=PARTIAL, 2=FULL)
#   i32  rpm
#   i32  speed              (kph)
#   f32  coolantTemp        (fahrenheit)
#   f32  fuelLevel          (0.0-1.0)
#   f32  throttlePosition   (0.0-1.0)
# < = little-endian, Q = u64, i = i32, f = f32
ENTRY_FORMAT = "<Qiiifff"
ENTRY_SIZE = struct.calcsize(ENTRY_FORMAT)

UPDATE_CODES = {0: "NONE", 1: "PARTIAL", 2: "FULL"}

HEADER = "timestamp_us,timestamp_s,update_result,rpm,speed_kph,speed_mph,coolant_temp_f,fuel_level,throttle_position"


def convert(input_path, output_path):
    with open(input_path, "rb") as f:
        data = f.read()

    entry_count = len(data) // ENTRY_SIZE
    remainder = len(data) % ENTRY_SIZE

    if remainder != 0:
        print(f"Warning: {remainder} trailing bytes (file may be truncated/corrupted)", file=sys.stderr)

    with open(output_path, "w") as out:
        out.write(HEADER + "\n")
        for i in range(entry_count):
            offset = i * ENTRY_SIZE
            entry = struct.unpack_from(ENTRY_FORMAT, data, offset)
            timestamp_us, update_code, rpm, speed, coolant, fuel, throttle = entry
            timestamp_s = timestamp_us / 1_000_000.0
            speed_mph = speed * 0.621371
            out.write(
                f"{timestamp_us},{timestamp_s:.6f},{UPDATE_CODES.get(update_code, update_code)},"
                f"{rpm},{speed},{speed_mph:.2f},{coolant:.2f},{fuel:.4f},{throttle:.4f}\n"
            )

    print(f"Converted {entry_count} entries: {output_path}")


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <input.bin> [output.csv]", file=sys.stderr)
        sys.exit(1)

    input_path = sys.argv[1]
    if len(sys.argv) >= 3:
        output_path = sys.argv[2]
    else:
        output_path = os.path.splitext(input_path)[0] + ".csv"

    convert(input_path, output_path)


if __name__ == "__main__":
    main()
