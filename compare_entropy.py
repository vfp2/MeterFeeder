#!/usr/bin/env python3
"""
compare_entropy.py — Side-by-side comparison of two MED RNG entropy sessions.

Produces a 2-column figure: left = session A (e.g. CE-5), right = session B (baseline).
Each column shows random walks, cross-correlation, GCP1 NetVar, and GCP2 coherence.

Usage:  python3 compare_entropy.py <dir_A> <dir_B> [label_A] [label_B]

Example:
    python3 compare_entropy.py ./entropy_data ./entropy_data_baseline "CE-5 Session" "Baseline"
"""

import os
import sys
import glob
import numpy as np
from datetime import datetime, timezone, timedelta
from itertools import combinations

NZDT = timezone(timedelta(hours=13))

from scipy import stats
from scipy.signal import hilbert, butter, sosfiltfilt

import matplotlib
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.gridspec as gridspec

COLORS = ["#1f77b4", "#ff7f0e", "#2ca02c", "#d62728", "#9467bd",
          "#8c564b", "#e377c2", "#7f7f7f", "#bcbd22"]

# ──────────────────────────────────────────────────────────────────────────────
# Optimized data loading — avoids datetime.strptime per line
# ──────────────────────────────────────────────────────────────────────────────

def _parse_ts_fast(line):
    """Extract epoch second (as int offset from midnight) and datetime from line.
    Line format: [2026-02-18T07:13:52.034933Z] [87.4ms] abcdef...
    Returns (datetime_truncated_to_second, hex_start_index) or None.
    """
    # Positions in "[YYYY-MM-DDThh:mm:ss.ffffffZ]"
    #               0123456789...
    if len(line) < 32 or line[0] != '[':
        return None
    try:
        Y = int(line[1:5])
        M = int(line[6:8])
        D = int(line[9:11])
        h = int(line[12:14])
        m = int(line[15:17])
        s = int(line[18:20])
        return datetime(Y, M, D, h, m, s, tzinfo=timezone.utc)
    except (ValueError, IndexError):
        return None


def _extract_hex(line):
    """Extract hex string from after the second '] '."""
    # Find the third segment after '] '
    i = line.find('] ', 1)  # end of timestamp
    if i < 0:
        return None
    i2 = line.find('] ', i + 2)  # end of ms
    if i2 < 0:
        return None
    return line[i2 + 2:].strip()


def load_session(data_dir):
    """
    Load all .hex files from a directory.
    Returns:
        serials: sorted list of serial strings
        epoch_times: array of datetime (one per second)
        z_matrix: (T, N) Z-scores per second per device
        walk_data: dict serial -> (timestamps_list, walk_positions_list)
    """
    hex_files = sorted(glob.glob(os.path.join(data_dir, "*.hex")))
    hex_files = [f for f in hex_files if not f.endswith('.png')]
    serials = [os.path.splitext(os.path.basename(f))[0] for f in hex_files]
    N = len(serials)

    print(f"  Loading {N} devices from {data_dir}...")

    # First pass: determine time range
    global_start = None
    global_end = None
    for fpath, serial in zip(hex_files, serials):
        with open(fpath, "r") as f:
            first_line = f.readline()
        ts = _parse_ts_fast(first_line)
        if ts:
            if global_start is None or ts < global_start:
                global_start = ts
        # Read last line efficiently
        with open(fpath, "rb") as f:
            f.seek(0, 2)
            fsize = f.tell()
            pos = max(0, fsize - 4096)
            f.seek(pos)
            last_lines = f.read().decode("utf-8", errors="replace").strip().split("\n")
        for ll in reversed(last_lines):
            ts = _parse_ts_fast(ll)
            if ts:
                if global_end is None or ts > global_end:
                    global_end = ts
                break

    total_seconds = int((global_end - global_start).total_seconds()) + 1
    print(f"  Time range: {global_start.strftime('%H:%M:%S')} - "
          f"{global_end.strftime('%H:%M:%S')} ({total_seconds}s)")

    epoch_times = np.array([global_start + timedelta(seconds=s)
                            for s in range(total_seconds)])

    z_matrix = np.full((total_seconds, N), np.nan)
    walk_data = {}

    for col, (fpath, serial) in enumerate(zip(hex_files, serials)):
        bit_sums = np.zeros(total_seconds)
        bit_counts = np.zeros(total_seconds, dtype=np.int64)
        # For random walk: track position per read
        walk_ts = []
        walk_pos = []
        pos = 0
        line_count = 0

        with open(fpath, "r") as f:
            for line in f:
                ts = _parse_ts_fast(line)
                if ts is None:
                    continue
                hex_str = _extract_hex(line)
                if hex_str is None:
                    continue

                epoch_idx = int((ts - global_start).total_seconds())
                if epoch_idx < 0 or epoch_idx >= total_seconds:
                    continue

                # Convert hex to bytes to bits efficiently
                raw = bytes.fromhex(hex_str)
                arr = np.frombuffer(raw, dtype=np.uint8)
                bits = np.unpackbits(arr)
                n_bits = len(bits)
                bit_sum = int(np.sum(bits))

                bit_sums[epoch_idx] += bit_sum
                bit_counts[epoch_idx] += n_bits

                # Walk: net steps = 2*sum - n  (each 1-bit = +1, each 0-bit = -1)
                pos += 2 * bit_sum - n_bits
                walk_ts.append(ts)
                walk_pos.append(pos)

                line_count += 1

        # Z-scores
        valid = bit_counts > 0
        n = bit_counts[valid].astype(np.float64)
        s = bit_sums[valid]
        z_matrix[valid, col] = (s - n / 2) / np.sqrt(n / 4)

        walk_data[serial] = (walk_ts, walk_pos)
        print(f"    {serial}: {line_count:,} reads")

    return serials, epoch_times, z_matrix, walk_data


# ──────────────────────────────────────────────────────────────────────────────
# Analysis functions (from analyze_entropy.py)
# ──────────────────────────────────────────────────────────────────────────────

def compute_netvar(z_matrix):
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
        "stouffer_z": stouffer_z, "netvar": netvar, "cumdev": cumdev,
        "chi2": chi2_total, "df": df, "p_value": p_value,
    }


def compute_correlation_matrix(z_matrix):
    N = z_matrix.shape[1]
    corr = np.full((N, N), np.nan)
    for i in range(N):
        for j in range(N):
            valid = ~np.isnan(z_matrix[:, i]) & ~np.isnan(z_matrix[:, j])
            if np.sum(valid) > 2:
                corr[i, j] = np.corrcoef(z_matrix[valid, i],
                                          z_matrix[valid, j])[0, 1]
    return corr


def bandpass(data, low, high, fs, order=4):
    nyq = 0.5 * fs
    sos = butter(order, [low / nyq, high / nyq], btype="band", output="sos")
    return sosfiltfilt(sos, data, axis=0)


def compute_coherence(z_matrix, fs=1.0, lowcut=0.01, highcut=0.1, win=60):
    T, N = z_matrix.shape
    z_filled = np.nan_to_num(z_matrix, nan=0.0)
    filtered = bandpass(z_filled, lowcut, highcut, fs)

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
    win_times = np.arange(n_windows) * win

    for w in range(n_windows):
        s, e = w * win, (w + 1) * win
        plv_sum = amp_sum = 0.0
        for (i, j) in pairs:
            phase_diff = phases[s:e, i] - phases[s:e, j]
            plv_sum += np.abs(np.mean(np.exp(1j * phase_diff)))
            a_i, a_j = amplitudes[s:e, i], amplitudes[s:e, j]
            if np.std(a_i) > 0 and np.std(a_j) > 0:
                amp_sum += np.corrcoef(a_i, a_j)[0, 1]
        plv_ts[w] = plv_sum / n_pairs
        amp_coh_ts[w] = amp_sum / n_pairs

    return {
        "plv": plv_ts, "amp_coherence": amp_coh_ts,
        "window_indices": win_times, "window_size": win, "n_windows": n_windows,
    }


# ──────────────────────────────────────────────────────────────────────────────
# Plotting helpers
# ──────────────────────────────────────────────────────────────────────────────

def plot_walks(ax, walk_data, serials, title):
    for i, serial in enumerate(serials):
        ts_list, pos_list = walk_data[serial]
        # Downsample for plotting if very large
        step = max(1, len(ts_list) // 50000)
        ax.plot(ts_list[::step], pos_list[::step],
                color=COLORS[i % len(COLORS)], linewidth=0.5,
                label=serial, alpha=0.85)
    ax.axhline(y=0, color="k", linewidth=0.4, linestyle="--")
    ax.set_ylabel("Cumulative Walk")
    ax.set_title(title, fontsize=10)
    ax.legend(fontsize=5.5, ncol=3, loc="upper left")
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M", tz=NZDT))
    ax.tick_params(axis="x", labelsize=7)


def plot_corr(ax, corr, serials, title):
    N = len(serials)
    vmax = max(0.02, np.nanmax(np.abs(corr[np.triu_indices(N, k=1)])))
    im = ax.imshow(corr, cmap="RdBu_r", vmin=-vmax, vmax=vmax, aspect="equal")
    plt.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
    short = [s[-4:] for s in serials]
    ax.set_xticks(range(N))
    ax.set_yticks(range(N))
    ax.set_xticklabels(short, fontsize=6, rotation=45, ha="right")
    ax.set_yticklabels(short, fontsize=6)
    for i in range(N):
        for j in range(N):
            if not np.isnan(corr[i, j]):
                ax.text(j, i, f"{corr[i, j]:.3f}", ha="center", va="center",
                        fontsize=4.5,
                        color="white" if abs(corr[i, j]) > vmax * 0.6 else "black")
    ax.set_title(title, fontsize=10)


def plot_nv(ax, epoch_times, nv, title):
    ax.plot(epoch_times, nv["cumdev"], color="#d62728", linewidth=0.7)
    ax.axhline(y=0, color="k", linewidth=0.4, linestyle="--")
    ax.fill_between(epoch_times, 0, nv["cumdev"],
                     where=np.array(nv["cumdev"]) > 0, alpha=0.12, color="#d62728")
    ax.fill_between(epoch_times, 0, nv["cumdev"],
                     where=np.array(nv["cumdev"]) < 0, alpha=0.12, color="#1f77b4")
    ax.set_ylabel("Cumulative Deviation", fontsize=8)
    ax.set_title(title, fontsize=10)
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M", tz=NZDT))
    ax.tick_params(axis="x", labelsize=7)


def plot_coh(ax, epoch_times, coh, title):
    win = coh["window_size"]
    win_times = [epoch_times[idx + win // 2]
                 for idx in coh["window_indices"]
                 if idx + win // 2 < len(epoch_times)]
    n = len(win_times)

    ax.plot(win_times[:n], coh["plv"][:n],
            color="#1f77b4", linewidth=0.7, label="PLV")
    ax.set_ylabel("PLV", color="#1f77b4", fontsize=8)
    ax.tick_params(axis="y", labelcolor="#1f77b4")
    ax.set_ylim(0, 1)

    ax2 = ax.twinx()
    ax2.plot(win_times[:n], coh["amp_coherence"][:n],
             color="#ff7f0e", linewidth=0.7, label="Amp. Coh.")
    ax2.set_ylabel("Amp. Coh. (r)", color="#ff7f0e", fontsize=8)
    ax2.tick_params(axis="y", labelcolor="#ff7f0e")
    ax2.set_ylim(-0.5, 1)

    ax.set_title(title, fontsize=10)
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M", tz=NZDT))
    ax.tick_params(axis="x", labelsize=7)
    lines1, labels1 = ax.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax.legend(lines1 + lines2, labels1 + labels2, fontsize=6, loc="upper right")


# ──────────────────────────────────────────────────────────────────────────────
# Main
# ──────────────────────────────────────────────────────────────────────────────

def main():
    if len(sys.argv) < 3:
        print("Usage: python3 compare_entropy.py <dir_A> <dir_B> [label_A] [label_B]")
        sys.exit(1)

    dir_a = sys.argv[1]
    dir_b = sys.argv[2]
    label_a = sys.argv[3] if len(sys.argv) > 3 else "Session A"
    label_b = sys.argv[4] if len(sys.argv) > 4 else "Session B"

    # ── Load ──
    print(f"\n{'='*60}")
    print(f"Loading {label_a}...")
    print(f"{'='*60}")
    ser_a, et_a, zm_a, wd_a = load_session(dir_a)

    print(f"\n{'='*60}")
    print(f"Loading {label_b}...")
    print(f"{'='*60}")
    ser_b, et_b, zm_b, wd_b = load_session(dir_b)

    # ── Compute ──
    print(f"\nComputing analyses for {label_a}...")
    nv_a = compute_netvar(zm_a)
    corr_a = compute_correlation_matrix(zm_a)
    coh_a = compute_coherence(zm_a)
    print(f"  NetVar: chi2={nv_a['chi2']:.1f}, df={nv_a['df']}, p={nv_a['p_value']:.6f}")
    print(f"  Mean PLV={np.mean(coh_a['plv']):.4f}, "
          f"Mean AmpCoh={np.mean(coh_a['amp_coherence']):.4f}")

    print(f"\nComputing analyses for {label_b}...")
    nv_b = compute_netvar(zm_b)
    corr_b = compute_correlation_matrix(zm_b)
    coh_b = compute_coherence(zm_b)
    print(f"  NetVar: chi2={nv_b['chi2']:.1f}, df={nv_b['df']}, p={nv_b['p_value']:.6f}")
    print(f"  Mean PLV={np.mean(coh_b['plv']):.4f}, "
          f"Mean AmpCoh={np.mean(coh_b['amp_coherence']):.4f}")

    # ── Summary stats for narrative ──
    off_diag_a = corr_a[np.triu_indices(len(ser_a), k=1)]
    off_diag_b = corr_b[np.triu_indices(len(ser_b), k=1)]

    print(f"\n{'='*60}")
    print(f"COMPARISON SUMMARY")
    print(f"{'='*60}")
    print(f"{'Metric':<30} {label_a:>18} {label_b:>18}")
    print(f"{'-'*66}")
    print(f"{'Duration (seconds)':<30} {zm_a.shape[0]:>18,} {zm_b.shape[0]:>18,}")
    print(f"{'Devices':<30} {len(ser_a):>18} {len(ser_b):>18}")
    print(f"{'NetVar chi2':<30} {nv_a['chi2']:>18.1f} {nv_b['chi2']:>18.1f}")
    print(f"{'NetVar df':<30} {nv_a['df']:>18} {nv_b['df']:>18}")
    print(f"{'NetVar p-value':<30} {nv_a['p_value']:>18.6f} {nv_b['p_value']:>18.6f}")
    print(f"{'Mean cross-correlation':<30} {np.nanmean(off_diag_a):>18.5f} "
          f"{np.nanmean(off_diag_b):>18.5f}")
    print(f"{'Max |cross-correlation|':<30} {np.nanmax(np.abs(off_diag_a)):>18.5f} "
          f"{np.nanmax(np.abs(off_diag_b)):>18.5f}")
    print(f"{'Mean PLV':<30} {np.mean(coh_a['plv']):>18.4f} "
          f"{np.mean(coh_b['plv']):>18.4f}")
    print(f"{'Mean Amp. Coherence':<30} {np.mean(coh_a['amp_coherence']):>18.4f} "
          f"{np.mean(coh_b['amp_coherence']):>18.4f}")
    print(f"{'NetVar final cumdev':<30} {nv_a['cumdev'][-1]:>18.1f} "
          f"{nv_b['cumdev'][-1]:>18.1f}")
    print(f"{'='*60}")

    # ── Plot: 4 rows x 2 columns ──
    print("\nPlotting comparison...")
    fig = plt.figure(figsize=(20, 20))
    fig.suptitle(f"MED RNG Coherence: {label_a} vs {label_b}",
                 fontsize=15, fontweight="bold", y=0.98)

    gs = gridspec.GridSpec(4, 2, height_ratios=[1.2, 1, 1, 1],
                           hspace=0.35, wspace=0.25)

    # Row 1: Random Walks
    ax_w_a = fig.add_subplot(gs[0, 0])
    plot_walks(ax_w_a, wd_a, ser_a, f"{label_a} — Random Walks")
    ax_w_b = fig.add_subplot(gs[0, 1])
    plot_walks(ax_w_b, wd_b, ser_b, f"{label_b} — Random Walks")

    # Row 2: Correlation matrices
    ax_c_a = fig.add_subplot(gs[1, 0])
    plot_corr(ax_c_a, corr_a, ser_a,
              f"{label_a} — Cross-Correlation")
    ax_c_b = fig.add_subplot(gs[1, 1])
    plot_corr(ax_c_b, corr_b, ser_b,
              f"{label_b} — Cross-Correlation")

    # Row 3: NetVar cumulative deviation
    ax_n_a = fig.add_subplot(gs[2, 0])
    plot_nv(ax_n_a, et_a, nv_a,
            f"{label_a} — NetVar "
            f"(\u03c7\u00b2={nv_a['chi2']:.0f}, df={nv_a['df']}, p={nv_a['p_value']:.4f})")
    ax_n_b = fig.add_subplot(gs[2, 1])
    plot_nv(ax_n_b, et_b, nv_b,
            f"{label_b} — NetVar "
            f"(\u03c7\u00b2={nv_b['chi2']:.0f}, df={nv_b['df']}, p={nv_b['p_value']:.4f})")

    # Match y-axis scale for NetVar panels
    ymin = min(ax_n_a.get_ylim()[0], ax_n_b.get_ylim()[0])
    ymax = max(ax_n_a.get_ylim()[1], ax_n_b.get_ylim()[1])
    ax_n_a.set_ylim(ymin, ymax)
    ax_n_b.set_ylim(ymin, ymax)

    # Row 4: Phase/amplitude coherence
    ax_p_a = fig.add_subplot(gs[3, 0])
    plot_coh(ax_p_a, et_a, coh_a, f"{label_a} — Phase & Amplitude Coherence")
    ax_p_b = fig.add_subplot(gs[3, 1])
    plot_coh(ax_p_b, et_b, coh_b, f"{label_b} — Phase & Amplitude Coherence")

    outpath = "coherence_comparison.png"
    plt.savefig(outpath, dpi=150, bbox_inches="tight")
    print(f"Saved to {outpath}")
    plt.show()


if __name__ == "__main__":
    main()
