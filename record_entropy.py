#!/usr/bin/env python3
"""
record_entropy.py — Continuously record hex entropy from every connected
MED USB RNG device into per-device files.

Usage:  python3 record_entropy.py [output_dir] [bytes_per_read]

  output_dir      Directory for output files (default: ./entropy_data)
  bytes_per_read  Bytes to read per iteration (default: 1024, max: 1048576)

Output files:  <output_dir>/<serial>.hex
Stop with:     Ctrl+C
"""

import os
import sys
import time
import signal
import threading
from datetime import datetime, timezone
from ctypes import (
    cdll, c_int, c_char_p, c_ubyte, create_string_buffer,
    addressof, POINTER,
)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LIB_PATH = os.path.join(SCRIPT_DIR, "builds", "linux", "libmeterfeeder.so")

OUTPUT_DIR = sys.argv[1] if len(sys.argv) >= 2 else "./entropy_data"
CHUNK = int(sys.argv[2]) if len(sys.argv) >= 3 else 1024

stop_event = threading.Event()


def load_library():
    lib = cdll.LoadLibrary(LIB_PATH)
    lib.MF_Initialize.argtypes = (c_char_p,)
    lib.MF_Initialize.restype = c_int
    lib.MF_Shutdown.argtypes = ()
    lib.MF_Shutdown.restype = None
    lib.MF_GetNumberGenerators.argtypes = ()
    lib.MF_GetNumberGenerators.restype = c_int
    lib.MF_GetListGenerators.argtypes = (POINTER(c_char_p),)
    lib.MF_GetBytes.argtypes = (c_int, POINTER(c_ubyte), c_char_p, c_char_p)
    return lib


def initialize(lib):
    err = create_string_buffer(256)
    result = lib.MF_Initialize(err)
    if err.value:
        print(f"MF_Initialize error: {err.value.decode()}")
        sys.exit(1)
    return result


def get_devices(lib):
    n = lib.MF_GetNumberGenerators()
    if n == 0:
        return {}
    bufs = [create_string_buffer(58) for _ in range(n)]
    ptrs = (c_char_p * n)(*map(addressof, bufs))
    lib.MF_GetListGenerators(ptrs)
    devices = {}
    for b in bufs:
        serial, desc = b.value.decode().split("|", 1)
        devices[serial] = desc
    return devices


def record_device(lib, serial, chunk, outpath):
    """Thread target: continuously read entropy and append hex to file."""
    buf = (c_ubyte * chunk)()
    err = create_string_buffer(256)
    reads = 0
    total_bytes = 0

    with open(outpath, "a") as f:
        while not stop_event.is_set():
            err.value = b""
            t_start = time.perf_counter()
            lib.MF_GetBytes(chunk, buf, serial.encode(), err)
            t_elapsed_ms = (time.perf_counter() - t_start) * 1000
            if err.value:
                print(f"  [{serial}] Error: {err.value.decode()}")
                time.sleep(0.5)
                continue
            timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%S.%fZ")
            hex_line = bytes(buf).hex()
            f.write(f"[{timestamp}] [{t_elapsed_ms:.1f}ms] {hex_line}\n")
            f.flush()
            reads += 1
            total_bytes += chunk

    print(f"  [{serial}] Stopped after {reads} reads ({total_bytes:,} bytes)")


def main():
    print("Loading library...")
    lib = load_library()

    print("Initializing driver...")
    initialize(lib)

    print("Discovering devices...")
    devices = get_devices(lib)
    if not devices:
        print("No MED devices found.")
        lib.MF_Shutdown()
        sys.exit(1)

    print(f"Found {len(devices)} device(s):")
    for serial, desc in devices.items():
        print(f"  {serial} ({desc})")

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    threads = []
    for serial, desc in devices.items():
        outpath = os.path.join(OUTPUT_DIR, f"{serial}.hex")
        print(f"  {serial} -> {outpath} ({CHUNK} bytes/read)")
        t = threading.Thread(
            target=record_device,
            args=(lib, serial, CHUNK, outpath),
            name=f"reader-{serial}",
            daemon=True,
        )
        t.start()
        threads.append(t)

    print(f"\nRecording entropy from {len(devices)} devices. Press Ctrl+C to stop.\n")

    def handle_signal(sig, frame):
        print("\nStopping...")
        stop_event.set()

    signal.signal(signal.SIGINT, handle_signal)
    signal.signal(signal.SIGTERM, handle_signal)

    # Wait for stop signal
    while not stop_event.is_set():
        time.sleep(0.5)

    # Wait for threads to finish their current read
    for t in threads:
        t.join(timeout=10)

    lib.MF_Shutdown()

    # Print final file sizes
    print(f"\nOutput files in: {OUTPUT_DIR}")
    for serial in devices:
        p = os.path.join(OUTPUT_DIR, f"{serial}.hex")
        sz = os.path.getsize(p) if os.path.exists(p) else 0
        print(f"  {serial}.hex: {sz:,} bytes")


if __name__ == "__main__":
    main()
