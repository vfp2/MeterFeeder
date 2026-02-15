#!/usr/bin/env bash
#
# record_entropy.sh — Continuously record hex entropy from every connected
# MED USB RNG device into per-device files.
#
# Usage:  ./record_entropy.sh [output_dir] [bytes_per_read]
#
#   output_dir      Directory for output files (default: ./entropy_data)
#   bytes_per_read  Bytes to read per iteration (default: 1024, max: 1048576)
#
# Output files:  <output_dir>/<serial>.hex
# Stop with:     Ctrl+C
#

set -euo pipefail

METERFEEDER="$(dirname "$0")/builds/linux/meterfeeder"
OUTDIR="${1:-./entropy_data}"
CHUNK="${2:-1024}"
PIDS=()

cleanup() {
    echo ""
    echo "Stopping all readers..."
    for pid in "${PIDS[@]}"; do
        kill "$pid" 2>/dev/null || true
    done
    wait 2>/dev/null
    echo "Done. Output files in: $OUTDIR"
    exit 0
}

trap cleanup SIGINT SIGTERM

if [[ ! -x "$METERFEEDER" ]]; then
    echo "Error: meterfeeder binary not found or not executable at: $METERFEEDER"
    exit 1
fi

# Discover connected devices by running meterfeeder with no args.
# Output format: "QWR4A003 (MED100K 100 kHz v1.0): 92"
echo "Discovering devices..."
DEVICE_OUTPUT=$("$METERFEEDER" 2>&1) || {
    echo "Error discovering devices: $DEVICE_OUTPUT"
    exit 1
}

# Extract serial numbers (QWR... tokens at the start of each line)
mapfile -t SERIALS < <(echo "$DEVICE_OUTPUT" | grep -oP '^QWR\S+')

if [[ ${#SERIALS[@]} -eq 0 ]]; then
    echo "No MED devices found."
    echo "Make sure:"
    echo "  1. Devices are plugged in"
    echo "  2. udev rules are installed (sudo ./linux-setup-udev.sh)"
    echo "  3. Your user is in the 'plugdev' group"
    exit 1
fi

echo "Found ${#SERIALS[@]} device(s): ${SERIALS[*]}"

mkdir -p "$OUTDIR"

# Launch a continuous reader for each device.
# The meterfeeder continuous mode outputs:
#   <hex bytes>\n\t====> N ms\n\n
# We filter out the timing lines and blank lines, keeping only raw hex.
for serial in "${SERIALS[@]}"; do
    outfile="$OUTDIR/${serial}.hex"
    echo "  $serial -> $outfile (${CHUNK} bytes/read)"

    "$METERFEEDER" "$serial" "$CHUNK" 1 2>&1 \
        | grep -a -v '====>' \
        | grep -a -v '^[[:space:]]*$' \
        >> "$outfile" &

    PIDS+=($!)
done

echo ""
echo "Recording entropy. Press Ctrl+C to stop."
echo ""

# Wait for all background processes; if any die unexpectedly, report it.
while true; do
    for i in "${!PIDS[@]}"; do
        pid="${PIDS[$i]}"
        if ! kill -0 "$pid" 2>/dev/null; then
            wait "$pid" 2>/dev/null
            rc=$?
            if [[ $rc -ne 0 ]]; then
                echo "Warning: reader for ${SERIALS[$i]} (PID $pid) exited with code $rc"
            fi
            unset 'PIDS[i]'
            unset 'SERIALS[i]'
        fi
    done
    # If all readers died, exit
    if [[ ${#PIDS[@]} -eq 0 ]]; then
        echo "All readers have stopped."
        exit 1
    fi
    sleep 2
done
