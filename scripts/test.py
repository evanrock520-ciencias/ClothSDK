#!/usr/bin/env python3
import argparse
import socket
import subprocess
import sys
from datetime import datetime
from pathlib import Path


def run_command(cmd, cwd=None):
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd)
    if result.returncode != 0:
        print(f"Command failed with exit code {result.returncode}")
        sys.exit(result.returncode)


def main():
    parser = argparse.ArgumentParser(description="Cross-platform run helper for Tissu")
    parser.add_argument("--no-test", action="store_true", help="Skip unit tests")
    parser.add_argument("--no-integration", action="store_true", help="Skip integration tests")
    parser.add_argument("--no-api-test", action="store_true", help="Skip API tests")
    parser.add_argument("--bench", action="store_true", help="Run benchmarks")

    args = parser.parse_args()

    # Paths
    script_dir = Path(__file__).parent.resolve()
    root_dir = script_dir.parent
    build_dir = root_dir / "build"

    # Helper to find executable
    def find_executable(name, subfolder):
        possible_paths = [
            build_dir / "bin" / "Release" / f"{name}.exe",
            build_dir / "bin" / "Release" / name,
            build_dir / "bin" / f"{name}.exe",
            build_dir / "bin" / name,
            build_dir / subfolder / "Release" / f"{name}.exe",
            build_dir / subfolder / "Release" / name,
            build_dir / subfolder / f"{name}.exe",
            build_dir / subfolder / name,
        ]
        for p in possible_paths:
            if p.is_file():
                return p
        print(
            f"Error: Executable '{name}' not found. Searched in: {[str(p) for p in possible_paths]}"  # noqa: E501
        )
        sys.exit(1)

    # 1. Test
    if not args.no_test:
        print("--- Running Unit Tests ---")
        exe = find_executable("unit_tests", "tests")
        run_command([str(exe)], cwd=root_dir)
        print()

    # 2. Integration Test
    if not args.no_integration:
        print("--- Running Integration Tests ---")
        exe = find_executable("integration", "tests")
        run_command([str(exe)], cwd=root_dir)
        print()

    # 3. API Tests
    if not args.no_api_test:
        print("--- Running API Tests ---")
        api_tests_path = root_dir / "tests" / "python"
        run_command(
            [sys.executable, "-m", "pytest", "-v", str(api_tests_path)],
            cwd=root_dir,
        )

    # 4. Benchmarks
    if args.bench:
        print("--- Running Benchmarks ---")
        exe = find_executable("tissu_benchmarks", "benchmarks")

        results_dir = root_dir / "benchmarks" / "results"
        results_dir.mkdir(parents=True, exist_ok=True)

        timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M")
        machine = socket.gethostname()
        output_file = results_dir / f"{timestamp}_{machine}.json"

        bench_cmd = [
            str(exe),
            "--benchmark_format=json",
            f"--benchmark_out={output_file}",
            "--benchmark_out_format=json",
            "--benchmark_repetitions=3",
            "--benchmark_report_aggregates_only=true",
        ]
        run_command(bench_cmd, cwd=root_dir)
        print(f"Results saved to {output_file}")
        print()


if __name__ == "__main__":
    main()
