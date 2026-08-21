"""
UWB Radar NPZ -- Range-Time-Amplitude (RTA) Plot

Kullanim:
    python visualize_npz.py                              # son .npz dosyasi
    python visualize_npz.py dosya.npz                    # belirli dosya
    python visualize_npz.py dosya.npz --clim -500 500    # renk araligi
    python visualize_npz.py dosya.npz --save             # PNG kaydet
    python visualize_npz.py dosya.npz --raw              # sadece ham veri (background subtraction yok)

Hesaplamalar:
    - 0-688 sample -> 7.2mm - 5m mesafe araligi
    - 14.53 FPS -> her satir ~68.8ms
"""

import sys
import os
import glob
import argparse
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import Normalize


def load_npz(filepath: str) -> dict:
    """NPZ dosyasini yukler."""
    data = np.load(filepath)
    frames = data["frames"].astype(np.float32)  # (N, max_samples)
    timestamps = data["timestamps"]
    sample_lengths = data["frame_sample_lengths"]

    max_len = int(sample_lengths.max())
    frames = frames[:, :max_len]

    return {
        "frames": frames,
        "timestamps": timestamps,
        "sample_lengths": sample_lengths,
        "filepath": filepath,
    }


def background_subtraction(frames: np.ndarray) -> np.ndarray:
    """Ortalama frame'i cikararak statik clutter'i kaldirir.
    Sadece zamana bagli degisimler (hareket, nefes, vs.) kalir."""
    mean_frame = frames.mean(axis=0)
    return frames - mean_frame


def compute_axes(n_frames: int, n_samples: int, timestamps: np.ndarray):
    """Range (metre) ve Time (saniye) eksenlerini hesaplar."""
    range_start_m = 0.0072  # 7.2 mm
    range_end_m = 5.0       # 5 m
    range_axis = np.linspace(range_start_m, range_end_m, n_samples)
    time_axis = timestamps - timestamps[0]
    return range_axis, time_axis


def plot_rta(data: dict, clim: tuple = None, save: bool = False, raw_only: bool = False):
    """Range-Time-Amplitude heatmap cizer."""
    frames_raw = data["frames"]
    timestamps = data["timestamps"]
    n_frames, n_samples = frames_raw.shape

    range_axis, time_axis = compute_axes(n_frames, n_samples, timestamps)
    duration = time_axis[-1]
    fps = n_frames / duration if duration > 0 else 0

    # Background subtraction
    frames_sub = background_subtraction(frames_raw)

    if raw_only:
        # Sadece ham veri
        fig, axes = plt.subplots(2, 1, figsize=(14, 10), height_ratios=[3, 1],
                                 gridspec_kw={"hspace": 0.3})
        _plot_heatmap(fig, axes[0], frames_raw, range_axis, time_axis, clim,
                      "Ham RTA (Raw)", "ADC Amplitude", "viridis")
        _plot_profile(axes[1], frames_raw, range_axis, time_axis)
    else:
        # 3 panel: ham + background subtracted + range profili
        fig, axes = plt.subplots(3, 1, figsize=(14, 14), height_ratios=[2, 2, 1],
                                 gridspec_kw={"hspace": 0.35})

        # 1) Ham veri
        _plot_heatmap(fig, axes[0], frames_raw, range_axis, time_axis, None,
                      "Ham RTA (Raw)", "ADC Amplitude", "viridis")

        # 2) Background subtracted -- zamana bagli degisim
        _plot_heatmap(fig, axes[1], frames_sub, range_axis, time_axis, clim,
                      "Background Subtracted (zamana bagli degisim)",
                      "Delta Amplitude", "RdBu_r")

        # 3) Zaman icerisinde belirli range bin'lerinin degisimi
        _plot_time_traces(axes[2], frames_sub, range_axis, time_axis)

    basename = os.path.splitext(os.path.basename(data["filepath"]))[0]
    fig.suptitle(f"{basename}  |  {n_frames} frames, {duration:.1f}s, {fps:.1f} FPS",
                 fontsize=11, y=0.99)
    plt.tight_layout()

    if save:
        out_path = os.path.splitext(data["filepath"])[0] + "_rta.png"
        fig.savefig(out_path, dpi=150, bbox_inches="tight")
        print(f"Kaydedildi: {out_path}")
    else:
        plt.show()


def _plot_heatmap(fig, ax, frames, range_axis, time_axis, clim, title, cbar_label, cmap):
    """Tek bir heatmap paneli cizer."""
    extent = [range_axis[0], range_axis[-1], time_axis[-1], time_axis[0]]

    if clim:
        norm = Normalize(vmin=clim[0], vmax=clim[1])
    else:
        p1, p99 = np.percentile(frames, [1, 99])
        norm = Normalize(vmin=p1, vmax=p99)

    im = ax.imshow(frames, aspect="auto", extent=extent, cmap=cmap, norm=norm,
                   interpolation="nearest")
    ax.set_xlabel("Range (m)")
    ax.set_ylabel("Time (s)")
    ax.set_title(title)
    fig.colorbar(im, ax=ax, label=cbar_label, pad=0.01)


def _plot_profile(ax, frames, range_axis, time_axis):
    """Ortadaki frame'in range profili."""
    mid = len(frames) // 2
    ax.plot(range_axis, frames[mid], linewidth=0.8, color="steelblue")
    ax.set_xlabel("Range (m)")
    ax.set_ylabel("ADC Amplitude")
    ax.set_title(f"Frame #{mid} profili (t = {time_axis[mid]:.2f}s)")
    ax.grid(True, alpha=0.3)


def _plot_time_traces(ax, frames_sub, range_axis, time_axis):
    """Belirli mesafelerdeki sinyal degisimini zaman ekseni uzerinde gosterir."""
    # 0.5m, 1m, 2m, 3m mesafelerindeki range bin'leri sec
    target_ranges = [0.5, 1.0, 2.0, 3.0]
    colors = ["#e74c3c", "#2ecc71", "#3498db", "#9b59b6"]

    for r, color in zip(target_ranges, colors):
        idx = np.argmin(np.abs(range_axis - r))
        actual_range = range_axis[idx]
        trace = frames_sub[:, idx]
        ax.plot(time_axis, trace, linewidth=0.8, color=color, alpha=0.8,
                label=f"{actual_range:.2f}m (bin {idx})")

    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Delta Amplitude")
    ax.set_title("Zamana bagli sinyal degisimi (belirli mesafeler)")
    ax.legend(loc="upper right", fontsize=8, ncol=4)
    ax.grid(True, alpha=0.3)


def find_latest_npz(output_dir: str) -> str:
    pattern = os.path.join(output_dir, "raw_collect_*.npz")
    files = sorted(glob.glob(pattern))
    if not files:
        print(f"Hata: {output_dir} klasorunde raw_collect_*.npz dosyasi bulunamadi.")
        sys.exit(1)
    return files[-1]


def main():
    parser = argparse.ArgumentParser(description="UWB Radar NPZ -> Range-Time-Amplitude")
    parser.add_argument("file", nargs="?", default=None, help=".npz dosya yolu")
    parser.add_argument("--clim", nargs=2, type=float, metavar=("MIN", "MAX"),
                        help="Background subtracted icin renk araligi (orn: --clim -500 500)")
    parser.add_argument("--save", action="store_true", help="PNG olarak kaydet")
    parser.add_argument("--raw", action="store_true", help="Sadece ham veri (background subtraction yok)")
    args = parser.parse_args()

    if args.file:
        filepath = args.file
    else:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        project_dir = os.path.dirname(script_dir)
        output_dir = os.path.join(project_dir, "output_data")
        filepath = find_latest_npz(output_dir)

    if not os.path.exists(filepath):
        print(f"Hata: Dosya bulunamadi: {filepath}")
        sys.exit(1)

    print(f"Dosya: {filepath}")

    data = load_npz(filepath)
    n_frames, n_samples = data["frames"].shape
    duration = data["timestamps"][-1] - data["timestamps"][0]

    print(f"  Frames : {n_frames}")
    print(f"  Samples: {n_samples}")
    print(f"  Sure   : {duration:.2f}s")
    print(f"  FPS    : {n_frames / duration:.1f}")

    clim = tuple(args.clim) if args.clim else None
    plot_rta(data, clim=clim, save=args.save, raw_only=args.raw)


if __name__ == "__main__":
    main()
