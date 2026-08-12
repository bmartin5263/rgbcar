#!/bin/sh -e

INPUT_DIR="src"
OUTPUT_DIR=".libsrc"
OUTPUT_BUILD_DIR=".libbuild"
INCLUDE_DIR="$OUTPUT_DIR/include"
SRC_DIR="$OUTPUT_DIR/src"

# Validate input directory
if [ ! -d "$INPUT_DIR" ]; then
  echo "Error: Input directory does not exist: $INPUT_DIR"
  exit 1
fi

# Remove and recreate output directory
rm -rf "$OUTPUT_DIR"
rm -rf "$OUTPUT_BUILD_DIR"
mkdir -p "$INCLUDE_DIR"
mkdir -p "$SRC_DIR"

# Function to determine if file is a C++ header
is_header() {
  case "$1" in
    *.h|*.hpp) return 0 ;;
    *) return 1 ;;
  esac
}

# Find and copy files, skipping directories intended for test-only or experimentation
find "$INPUT_DIR" -type d -name "main" -prune -o -type f ! -name "main.ino" -print | while read -r file; do
  base_name=$(basename "$file")

  if is_header "$base_name"; then
    dest="$INCLUDE_DIR/$base_name"
  else
    dest="$SRC_DIR/$base_name"
  fi

  if [ -e "$dest" ]; then
    echo "Error: Duplicate file name '$base_name' in destination: $dest"
    echo "Aborting Operation"
    rm -rf "$OUTPUT_DIR"
    exit 1
  fi

  cp "$file" "$dest"
done

cp "library.json" "$OUTPUT_DIR"
mkdir -p "$OUTPUT_BUILD_DIR"
pio pkg pack "$OUTPUT_DIR" -o "$OUTPUT_BUILD_DIR/rgbcar.tar.gz"