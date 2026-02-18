#!/usr/bin/env python3
"""
analyze_entropy.py — Coherence analysis of multi-device MED RNG entropy data.

Reads timestamped hex entropy files produced by record_entropy.py and generates
a 4-panel figure:
  1. Random walks (overlay, all devices)
  2. Cross-correlation matrix (heatmap)
  3. GCP1 Network Variance cumulative deviation
  4. GCP2-style phase & amplitude coherence

Usage:  python3 analyze_entropy.py [entropy_data_dir]
"""

import os
import sys
import glob
import numpy as np
from datetime import datetime, timezone
from itertools import combinations

from scipy import stats
from scipy.signal import hilbert, butter, sosfiltfilt

import matplotlib
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.gridspec as gridspec

# ──────────────────────────────────────────────────────────────────────────────
# Data loading
# ──────────────────────────────────────────────────────────────────────────────

def parse_hex_file(filepath):
    """Parse a .hex file into lists of (datetime, raw_bytes)."""
    timestamps = []
    byte_chunks = []
    with open(filepath, "r") as f:
        for line in f:
            line = line.strip()
            if not line or not line.startswith("["):
                continue
            # Format: [2026-02-15T07:30:01.729037Z] [87.4ms] abcdef...
            try:
                parts = line.split("] ")
                ts_str = parts[0].lstrip("[")
                hex_str = parts[2] if len(parts) >= 3 else parts[-1]
                ts = datetime.strptime(ts_str, "%Y-%m-%dT%H:%M:%S.%fZ").replace(
                    tzinfo=timezone.utc
                )
                raw = bytes.fromhex(hex_str)
                timestamps.append(ts)
                byte_chunks.append(raw)
            except (ValueError, IndexError):
                continue
    return timestamps, byte_chunks


def bytes_to_bits(raw_bytes):
    """Convert bytes to a numpy array of bits (0/1)."""
    arr = np.frombuffer(raw_bytes, dtype=np.uint8)
    return np.unpackbits(arr)


def load_all_devices(data_dir):
    """Load all .hex files. Returns dict: serial -> (timestamps, byte_chunks)."""
    devices = {}
    for fpath in sorted(glob.glob(os.path.join(data_dir, "*.hex"))):
        serial = os.path.splitext(os.path.basename(fpath))[0]
        ts, chunks = parse_hex_file(fpath)
        if ts:
            devices[serial] = (ts, chunks)
            print(f"  {serial}: {len(ts)} reads, "
                  f"{ts[0].strftime('%H:%M:%S')} - {ts[-1].strftime('%H:%M:%S')}")
    return devices


# ──────────────────────────────────────────────────────────────────────────────
# Time-aligned epoch matrix
# ──────────────────────────────────────────────────────────────────────────────

def build_epoch_matrix(devices):
    """
    Bin all device data into 1-second epochs.

    Returns:
        epoch_times: array of datetime objects, one per second
        z_matrix: (T, N) float array of per-second Z-scores (NaN where no data)
        serials: list of serial strings in column order
    """
    serials = sorted(devices.keys())
    N = len(serials)

    # Determine global time range
    all_starts = [devices[s][0][0] for s in serials]
    all_ends = [devices[s][0][-1] for s in serials]
    t_start = min(all_starts).replace(microsecond=0)
    t_end = max(all_ends).replace(microsecond=0)

    total_seconds = int((t_end - t_start).total_seconds()) + 1
    epoch_times = [t_start + __import__("datetime").timedelta(seconds=s)
                   for s in range(total_seconds)]

    z_matrix = np.full((total_seconds, N), np.nan)

    for col, serial in enumerate(serials):
        timestamps, byte_chunks = devices[serial]
        # Bin bits into 1-second buckets
        bit_sums = np.zeros(total_seconds)
        bit_counts = np.zeros(total_seconds, dtype=np.int64)

        for ts, chunk in zip(timestamps, byte_chunks):
            epoch_idx = int((ts - t_start).total_seconds())
            if 0 <= epoch_idx < total_seconds:
                bits = bytes_to_bits(chunk)
                bit_sums[epoch_idx] += np.sum(bits)
                bit_counts[epoch_idx] += len(bits)

        # Compute Z-scores where we have data
        valid = bit_counts > 0
        n = bit_counts[valid].astype(np.float64)
        s = bit_sums[valid]
        z_matrix[valid, col] = (s - n / 2) / np.sqrt(n / 4)

    return np.array(epoch_times), z_matrix, serials


# ──────────────────────────────────────────────────────────────────────────────
# Panel 1: Random walks
# ──────────────────────────────────────────────────────────────────────────────

COLORS = ["#1f77b4", "#ff7f0e", "#2ca02c", "#d62728", "#9467bd",
          "#8c564b", "#e377c2", "#7f7f7f", "#bcbd22"]


def plot_random_walks(ax, devices, serials):
    """Overlay random walk for each device, x-axis = real timestamps."""
    for i, serial in enumerate(serials):
        timestamps, byte_chunks = devices[serial]
        # Build walk: accumulate bits as +1/-1
        walk_segments_x = []
        walk_segments_y = []
        pos = 0
        for ts, chunk in zip(timestamps, byte_chunks):
            bits = bytes_to_bits(chunk)
            steps = bits.astype(np.int32) * 2 - 1  # 0->-1, 1->+1
            cumulative = np.cumsum(steps) + pos
            pos = cumulative[-1]
            # Use the timestamp for the whole chunk (sub-second resolution not needed)
            walk_segments_x.append(ts)
            walk_segments_y.append(pos)

        ax.plot(walk_segments_x, walk_segments_y,
                color=COLORS[i % len(COLORS)], linewidth=0.6,
                label=serial, alpha=0.85)

    ax.axhline(y=0, color="k", linewidth=0.5, linestyle="--")
    ax.set_ylabel("Cumulative Walk")
    ax.set_title("Random Walks — All Devices")
    ax.legend(fontsize=7, ncol=3, loc="upper left")
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))
    ax.tick_params(axis="x", labelsize=8)


# ──────────────────────────────────────────────────────────────────────────────
# Panel 2: Cross-correlation matrix
# ──────────────────────────────────────────────────────────────────────────────

def plot_correlation_matrix(ax, z_matrix, serials):
    """Pairwise Pearson correlation heatmap of per-second Z-scores."""
    N = len(serials)
    corr = np.full((N, N), np.nan)

    for i in range(N):
        for j in range(N):
            valid = ~np.isnan(z_matrix[:, i]) & ~np.isnan(z_matrix[:, j])
            if np.sum(valid) > 2:
                corr[i, j] = np.corrcoef(z_matrix[valid, i], z_matrix[valid, j])[0, 1]

    vmax = max(0.05, np.nanmax(np.abs(corr[np.triu_indices(N, k=1)])))
    im = ax.imshow(corr, cmap="RdBu_r", vmin=-vmax, vmax=vmax, aspect="equal")
    plt.colorbar(im, ax=ax, fraction=0.046, pad=0.04)

    ax.set_xticks(range(N))
    ax.set_yticks(range(N))
    short = [s[-4:] for s in serials]  # last 4 chars for readability
    ax.set_xticklabels(short, fontsize=7, rotation=45, ha="right")
    ax.set_yticklabels(short, fontsize=7)

    # Annotate cells
    for i in range(N):
        for j in range(N):
            if not np.isnan(corr[i, j]):
                ax.text(j, i, f"{corr[i, j]:.3f}", ha="center", va="center",
                        fontsize=5.5,
                        color="white" if abs(corr[i, j]) > vmax * 0.6 else "black")

    ax.set_title("Cross-Correlation (Z-scores)")


# ──────────────────────────────────────────────────────────────────────────────
# Panel 3: GCP1 Network Variance
# ──────────────────────────────────────────────────────────────────────────────

def compute_netvar(z_matrix):
    """
    GCP1 Network Variance analysis.

    Returns dict with stouffer_z, netvar, cumdev, chi2, df, p_value.
    """
    T, N = z_matrix.shape

    stouffer_z = np.full(T, np.nan)
    for t in range(T):
        row = z_matrix[t, :]
        valid = ~np.isnan(row)
        n_active = np.sum(valid)
        if n_active >= 2:
            stouffer_z[t] = np.sum(row[valid]) / np.sqrt(n_active)

    valid_t = ~np.isnan(stouffer_z)
    netvar = np.full(T, np.nan)
    netvar[valid_t] = stouffer_z[valid_t] ** 2

    # Cumulative deviation from expectation (E[chi2(1)] = 1)
    cumdev = np.zeros(T)
    running = 0.0
    for t in range(T):
        if valid_t[t]:
            running += netvar[t] - 1.0
        cumdev[t] = running

    df = int(np.sum(valid_t))
    chi2_total = float(np.nansum(netvar[valid_t]))
    p_value = 1.0 - stats.chi2.cdf(chi2_total, df)

    return {
        "stouffer_z": stouffer_z,
        "netvar": netvar,
        "cumdev": cumdev,
        "chi2": chi2_total,
        "df": df,
        "p_value": p_value,
    }


def plot_netvar(ax, epoch_times, nv):
    """Plot GCP1 cumulative deviation."""
    ax.plot(epoch_times, nv["cumdev"], color="#d62728", linewidth=0.8)
    ax.axhline(y=0, color="k", linewidth=0.5, linestyle="--")
    ax.fill_between(epoch_times, 0, nv["cumdev"],
                     where=np.array(nv["cumdev"]) > 0,
                     alpha=0.15, color="#d62728")
    ax.fill_between(epoch_times, 0, nv["cumdev"],
                     where=np.array(nv["cumdev"]) < 0,
                     alpha=0.15, color="#1f77b4")
    ax.set_ylabel("Cumulative Deviation")
    ax.set_title(
        f"GCP1 NetVar — "
        f"\u03c7\u00b2={nv['chi2']:.1f}, df={nv['df']}, p={nv['p_value']:.4f}"
    )
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))
    ax.tick_params(axis="x", labelsize=8)


# ──────────────────────────────────────────────────────────────────────────────
# Panel 4: GCP2-style phase & amplitude coherence
# ──────────────────────────────────────────────────────────────────────────────

def bandpass(data, low, high, fs, order=4):
    """Butterworth bandpass filter along axis 0."""
    nyq = 0.5 * fs
    sos = butter(order, [low / nyq, high / nyq], btype="band", output="sos")
    return sosfiltfilt(sos, data, axis=0)


def compute_coherence(z_matrix, fs=1.0, lowcut=0.01, highcut=0.1, win=60):
    """
    Phase Locking Value and amplitude coherence in sliding windows.

    Requires gap-free Z-score data; NaNs are zero-filled before filtering.
    """
    T, N = z_matrix.shape
    # Fill NaN with 0 for filtering (gaps become neutral)
    z_filled = np.nan_to_num(z_matrix, nan=0.0)

    # Bandpass filter
    filtered = bandpass(z_filled, lowcut, highcut, fs)

    # Analytic signal via Hilbert transform
    amplitudes = np.zeros((T, N))
    phases = np.zeros((T, N))
    for i in range(N):
        analytic = hilbert(filtered[:, i])
        amplitudes[:, i] = np.abs(analytic)
        phases[:, i] = np.angle(analytic)

    pairs = list(combinations(range(N), 2))
    n_pairs = len(pairs)
    n_windows = T // win

    plv_ts = np.zeros(n_windows)
    amp_coh_ts = np.zeros(n_windows)
    win_times = np.arange(n_windows) * win  # index offsets

    for w in range(n_windows):
        s = w * win
        e = s + win

        # Phase Locking Value
        plv_sum = 0.0
        amp_sum = 0.0
        for (i, j) in pairs:
            phase_diff = phases[s:e, i] - phases[s:e, j]
            plv_sum += np.abs(np.mean(np.exp(1j * phase_diff)))

            # Amplitude coherence (Pearson r of envelopes)
            a_i = amplitudes[s:e, i]
            a_j = amplitudes[s:e, j]
            if np.std(a_i) > 0 and np.std(a_j) > 0:
                amp_sum += np.corrcoef(a_i, a_j)[0, 1]

        plv_ts[w] = plv_sum / n_pairs
        amp_coh_ts[w] = amp_sum / n_pairs

    return {
        "plv": plv_ts,
        "amp_coherence": amp_coh_ts,
        "window_indices": win_times,
        "window_size": win,
        "n_windows": n_windows,
    }


def plot_coherence(ax, epoch_times, coh):
    """Plot phase and amplitude coherence time series."""
    win = coh["window_size"]
    # Center of each window as timestamp
    win_times = [epoch_times[idx + win // 2]
                 for idx in coh["window_indices"]
                 if idx + win // 2 < len(epoch_times)]
    n = len(win_times)

    ax.plot(win_times[:n], coh["plv"][:n],
            color="#1f77b4", linewidth=0.8, label="Phase Locking Value")
    ax.set_ylabel("PLV", color="#1f77b4")
    ax.tick_params(axis="y", labelcolor="#1f77b4")
    ax.set_ylim(0, 1)

    ax2 = ax.twinx()
    ax2.plot(win_times[:n], coh["amp_coherence"][:n],
             color="#ff7f0e", linewidth=0.8, label="Amplitude Coherence")
    ax2.set_ylabel("Amp. Coherence (r)", color="#ff7f0e")
    ax2.tick_params(axis="y", labelcolor="#ff7f0e")
    ax2.set_ylim(-0.5, 1)

    ax.set_title("GCP2-Style Coherence (bandpass 0.01–0.1 Hz, 60s windows)")
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))
    ax.tick_params(axis="x", labelsize=8)

    # Combined legend
    lines1, labels1 = ax.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax.legend(lines1 + lines2, labels1 + labels2, fontsize=7, loc="upper right")


# ──────────────────────────────────────────────────────────────────────────────
# Main
# ──────────────────────────────────────────────────────────────────────────────

def main():
    data_dir = sys.argv[1] if len(sys.argv) >= 2 else "./entropy_data"

    print("Loading data...")
    devices = load_all_devices(data_dir)
    if not devices:
        print("No .hex files found in", data_dir)
        sys.exit(1)

    serials = sorted(devices.keys())
    print(f"\n{len(serials)} devices loaded. Building epoch matrix...")
    epoch_times, z_matrix, serials = build_epoch_matrix(devices)
    T, N = z_matrix.shape
    print(f"Epoch matrix: {T} seconds x {N} devices\n")

    # ── Compute analyses ──
    print("Computing GCP1 NetVar...")
    nv = compute_netvar(z_matrix)
    print(f"  chi2={nv['chi2']:.1f}, df={nv['df']}, p={nv['p_value']:.6f}")

    print("Computing phase/amplitude coherence...")
    coh = compute_coherence(z_matrix, fs=1.0, lowcut=0.01, highcut=0.1, win=60)
    print(f"  {coh['n_windows']} windows, mean PLV={np.mean(coh['plv']):.4f}, "
          f"mean AmpCoh={np.mean(coh['amp_coherence']):.4f}")

    # ── Plot ──
    print("Plotting...")
    fig = plt.figure(figsize=(16, 14))
    fig.suptitle("MED RNG Network Coherence Analysis", fontsize=14, fontweight="bold")

    gs = gridspec.GridSpec(3, 2, height_ratios=[1.2, 1, 1],
                           hspace=0.35, wspace=0.3)

    # Panel 1: Random walks (top, full width)
    ax1 = fig.add_subplot(gs[0, :])
    plot_random_walks(ax1, devices, serials)

    # Panel 2: Correlation matrix (middle left)
    ax2 = fig.add_subplot(gs[1, 0])
    plot_correlation_matrix(ax2, z_matrix, serials)

    # Panel 3: NetVar (middle right)
    ax3 = fig.add_subplot(gs[1, 1])
    plot_netvar(ax3, epoch_times, nv)

    # Panel 4: Phase/amplitude coherence (bottom, full width)
    ax4 = fig.add_subplot(gs[2, :])
    plot_coherence(ax4, epoch_times, coh)

    plt.savefig(os.path.join(data_dir, "coherence_analysis.png"), dpi=150,
                bbox_inches="tight")
    print(f"Saved to {data_dir}/coherence_analysis.png")
    plt.show()


if __name__ == "__main__":
    main()
