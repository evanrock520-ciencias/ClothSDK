#!/usr/bin/env python3
"""
Script to parse Google Benchmark JSON output files and generate plots for Tissu.
Supports plotting Execution Time (CPU / Real time) and Throughput (Items per second)
against parameters like Number of Particles or Grid Size.
"""

import argparse
import json
from pathlib import Path

import matplotlib.pyplot as plt


def find_latest_json(results_dir: Path) -> Path:
    """Find the most recent .json file in the results directory."""
    json_files = list(results_dir.glob("*.json"))
    if not json_files:
        raise FileNotFoundError(f"No JSON benchmark files found in {results_dir}")
    return max(json_files, key=lambda f: f.stat().st_mtime)


def load_json_lenient(file_path: Path) -> dict:
    """Load JSON file with fallback recovery for truncated files."""
    content = file_path.read_text(encoding="utf-8").strip()
    try:
        return json.loads(content)
    except json.JSONDecodeError:
        # Attempt auto-closing brackets for truncated JSON
        for suffix in ["\n  ]\n}", "\n}", "]}\n", "}"]:
            try:
                return json.loads(content + suffix)
            except json.JSONDecodeError:
                continue
        raise


def parse_benchmark_json(file_path: Path):
    """
    Parse Google Benchmark JSON data and group aggregate metrics (mean, stddev).
    Returns a dictionary of benchmark family runs with parsed parameters and metrics.
    """
    data = load_json_lenient(file_path)

    context = data.get("context", {})
    benchmarks = data.get("benchmarks", [])

    # Group benchmarks by run_name or family_index
    runs = {}

    for bm in benchmarks:
        run_name = bm.get("run_name") or bm.get("name")
        agg_name = bm.get("aggregate_name")  # 'mean', 'median', 'stddev', 'cv', or None

        # Strip aggregate suffix from run_name if present
        if not run_name and "/" in bm["name"]:
            run_name = bm["name"].rsplit("_", 1)[0]

        if run_name not in runs:
            runs[run_name] = {}

        if agg_name:
            runs[run_name][agg_name] = bm
        else:
            # Non-aggregate single run
            runs[run_name]["single"] = bm

    parsed_data = []

    for run_name, aggs in runs.items():
        # Prefer mean entry, fallback to single run
        base_entry = aggs.get("mean") or aggs.get("single") or list(aggs.values())[0]
        stddev_entry = aggs.get("stddev")

        grid_size = float(base_entry.get("GridSize", 0.0))
        particles = float(base_entry.get("ParticlesPerFrame", 0.0))
        real_time = float(base_entry.get("real_time", 0.0))
        cpu_time = float(base_entry.get("cpu_time", 0.0))
        time_unit = base_entry.get("time_unit", "ms")
        items_per_sec = float(base_entry.get("items_per_second", 0.0))

        real_time_stddev = float(stddev_entry.get("real_time", 0.0)) if stddev_entry else 0.0
        cpu_time_stddev = float(stddev_entry.get("cpu_time", 0.0)) if stddev_entry else 0.0
        items_stddev = float(stddev_entry.get("items_per_second", 0.0)) if stddev_entry else 0.0

        parsed_data.append(
            {
                "run_name": run_name,
                "grid_size": grid_size,
                "particles": particles,
                "real_time": real_time,
                "real_time_stddev": real_time_stddev,
                "cpu_time": cpu_time,
                "cpu_time_stddev": cpu_time_stddev,
                "time_unit": time_unit,
                "items_per_sec": items_per_sec,
                "items_stddev": items_stddev,
            }
        )

    # Sort data by Particles then GridSize
    parsed_data.sort(key=lambda x: (x["particles"], x["grid_size"]))
    return context, parsed_data


def plot_benchmarks(context: dict, data: list, output_prefix: Path, x_axis: str = "particles", show: bool = False):
    """Generate and save benchmark visualization plots."""
    if not data:
        print("No benchmark data available to plot.")
        return

    # Modern visual styling
    plt.style.use("seaborn-v0_8-whitegrid" if "seaborn-v0_8-whitegrid" in plt.style.available else "default")
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

    if x_axis == "particles":
        x_values = [int(d["particles"]) if d["particles"].is_integer() else d["particles"] for d in data]
        x_label = "Number of Particles"
        title_suffix = "vs Number of Particles"
    else:
        x_values = [int(d["grid_size"]) if d["grid_size"].is_integer() else d["grid_size"] for d in data]
        x_label = "Grid Size"
        title_suffix = "vs Grid Size"

    # Fallbacks if selected metric has zero values
    if all(x == 0 for x in x_values):
        x_values = list(range(len(data)))
        x_label = "Benchmark Index"
        title_suffix = "vs Benchmark Index"

    # Plot 1: Execution Time
    real_times = [d["real_time"] for d in data]
    real_errs = [d["real_time_stddev"] for d in data]
    cpu_times = [d["cpu_time"] for d in data]
    cpu_errs = [d["cpu_time_stddev"] for d in data]
    time_unit = data[0]["time_unit"]

    ax1.errorbar(
        x_values,
        real_times,
        yerr=real_errs,
        fmt="-o",
        color="#2b5c8f",
        capsize=4,
        label=f"Real Time ({time_unit})",
        linewidth=2.5,
        markersize=7,
    )
    ax1.errorbar(
        x_values,
        cpu_times,
        yerr=cpu_errs,
        fmt="--s",
        color="#e07a5f",
        capsize=4,
        label=f"CPU Time ({time_unit})",
        linewidth=2.5,
        markersize=7,
    )

    ax1.set_title(f"Solver Execution Time {title_suffix}", fontsize=13, fontweight="bold", pad=12)
    ax1.set_xlabel(x_label, fontsize=11, fontweight="bold")
    ax1.set_ylabel(f"Time ({time_unit})", fontsize=11, fontweight="bold")
    ax1.legend(fontsize=10, frameon=True)
    ax1.set_xticks(x_values)
    ax1.grid(True, linestyle="--", alpha=0.6)

    # Annotate values on ax1
    for x, y in zip(x_values, real_times, strict=False):
        ax1.annotate(
            f"{y:.1f} {time_unit}",
            (x, y),
            textcoords="offset points",
            xytext=(0, 10),
            ha="center",
            fontsize=9,
            fontweight="bold",
        )

    # Plot 2: Throughput (Items / Second)
    items_sec = [d["items_per_sec"] for d in data]
    items_errs = [d["items_stddev"] for d in data]

    ax2.errorbar(
        x_values,
        items_sec,
        yerr=items_errs,
        fmt="-^",
        color="#2a9d8f",
        capsize=4,
        label="Throughput (items/s)",
        linewidth=2.5,
        markersize=7,
    )

    ax2.set_title(f"Solver Throughput {title_suffix}", fontsize=13, fontweight="bold", pad=12)
    ax2.set_xlabel(x_label, fontsize=11, fontweight="bold")
    ax2.set_ylabel("Items / Second", fontsize=11, fontweight="bold")
    ax2.legend(fontsize=10, frameon=True)
    ax2.set_xticks(x_values)
    ax2.grid(True, linestyle="--", alpha=0.6)

    # Annotate values on ax2
    for x, y in zip(x_values, items_sec, strict=False):
        ax2.annotate(
            f"{y:.1f}", (x, y), textcoords="offset points", xytext=(0, 10), ha="center", fontsize=9, fontweight="bold"
        )

    # Metadata Title
    date_str = context.get("date", "")
    host_str = context.get("host_name", "")
    cpus_str = context.get("num_cpus", "")
    fig.suptitle(
        f"Tissu Solver Benchmark - {host_str} ({cpus_str} CPUs) | {date_str}", fontsize=12, color="#555555", y=0.98
    )

    plt.tight_layout()

    # Save figure
    output_png = output_prefix.with_suffix(".png")
    plt.savefig(output_png, dpi=300, bbox_inches="tight")
    print(f"Plot saved successfully to: {output_png}")

    if show:
        plt.show()

    plt.close()


def main():
    parser = argparse.ArgumentParser(description="Plot Google Benchmark results for Tissu.")
    parser.add_argument("-f", "--file", type=Path, help="Path to Google Benchmark JSON output file.")
    parser.add_argument(
        "-l",
        "--latest",
        action="store_true",
        help="Automatically process the latest JSON result in benchmarks/results/.",
    )
    parser.add_argument(
        "-x",
        "--x-axis",
        choices=["particles", "grid"],
        default="particles",
        help="Variable to use on the X axis (particles or grid). Default: particles.",
    )
    parser.add_argument("--show", action="store_true", help="Display the plot window after generating.")

    args = parser.parse_args()
    script_dir = Path(__file__).parent
    results_dir = script_dir / "results"

    if args.file:
        json_file = args.file
    elif args.latest or not args.file:
        json_file = find_latest_json(results_dir)

    print(f"Processing benchmark results from: {json_file}")
    context, data = parse_benchmark_json(json_file)
    plot_benchmarks(context, data, json_file, x_axis=args.x_axis, show=args.show)


if __name__ == "__main__":
    main()
