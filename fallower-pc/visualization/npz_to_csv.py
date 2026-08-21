"""
UWB Radar NPZ -> CSV Donusturucu

Kullanim:
    python npz_to_csv.py                                    # son .npz dosyasi
    python npz_to_csv.py dosya.npz                          # belirli dosya
    python npz_to_csv.py dosya.npz --output cikti.csv       # cikti yolu belirt

Cikti CSV formati:
    frame_id, timestamp, stm32_timestamp, duration, sample_count, s0, s1, s2, ..., s687
"""

import sys
import os
import glob
import argparse
import numpy as np


def find_latest_npz(output_dir: str) -> str:
    pattern = os.path.join(output_dir, "raw_collect_*.npz")
    files = sorted(glob.glob(pattern))
    if not files:
        print(f"Hata: {output_dir} klasorunde raw_collect_*.npz dosyasi bulunamadi.")
        sys.exit(1)
    return files[-1]


def npz_to_csv(npz_path: str, csv_path: str):
    data = np.load(npz_path)

    frames = data["frames"]                     # (N, max_samples) int16
    timestamps = data["timestamps"]             # (N,) float64
    frame_ids = data["frame_ids"]               # (N,) uint32
    stm32_timestamps = data["stm32_timestamps"] # (N,) uint32
    durations = data["durations"]               # (N,) uint32
    sample_counts = data["sample_counts"]       # (N,) uint16

    n_frames, n_samples = frames.shape

    # Header
    header_fields = ["frame_id", "timestamp", "stm32_timestamp", "duration", "sample_count"]
    header_fields += [f"s{i}" for i in range(n_samples)]
    header = ",".join(header_fields)

    # Meta kolonlari birlestir
    meta = np.column_stack([
        frame_ids.astype(np.float64),
        timestamps,
        stm32_timestamps.astype(np.float64),
        durations.astype(np.float64),
        sample_counts.astype(np.float64),
    ])

    # Tum veriyi birlestir
    full = np.column_stack([meta, frames.astype(np.float64)])

    # Kaydet
    np.savetxt(csv_path, full, delimiter=",", header=header, comments="",
               fmt=["%d", "%.6f", "%d", "%d", "%d"] + ["%d"] * n_samples)

    print(f"Donusturuldu: {npz_path}")
    print(f"  Frames : {n_frames}")
    print(f"  Samples: {n_samples}")
    print(f"  CSV    : {csv_path}")
    print(f"  Boyut  : {os.path.getsize(csv_path) / (1024*1024):.1f} MB")


def main():
    parser = argparse.ArgumentParser(description="UWB Radar NPZ -> CSV")
    parser.add_argument("file", nargs="?", default=None, help=".npz dosya yolu")
    parser.add_argument("--output", "-o", default=None, help="Cikti CSV dosya yolu")
    args = parser.parse_args()

    if args.file:
        npz_path = args.file
    else:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        project_dir = os.path.dirname(script_dir)
        output_dir = os.path.join(project_dir, "output_data")
        npz_path = find_latest_npz(output_dir)

    if not os.path.exists(npz_path):
        print(f"Hata: Dosya bulunamadi: {npz_path}")
        sys.exit(1)

    if args.output:
        csv_path = args.output
    else:
        csv_path = os.path.splitext(npz_path)[0] + ".csv"

    npz_to_csv(npz_path, csv_path)


if __name__ == "__main__":
    main()
