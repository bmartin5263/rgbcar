#!/bin/sh -e

root="$(cd "$(dirname "$0")/.." && pwd)"

pio run -e release

for example in "$root"/examples/*/; do
  pio run -d "$example" -e nano_esp32
done