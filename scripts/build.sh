#!/bin/sh -e

pio run -e release

for example in "$root"/examples/*/; do
  pio run -d "$example" -e nano_esp32
done