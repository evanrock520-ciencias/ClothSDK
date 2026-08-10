#!/usr/bin/env python3
import argparse
import platform
import subprocess
import sys
from pathlib import Path


def run_command(cmd, cwd=None):
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd)
    if result.returncode != 0:
        print(f"Command failed with exit code {result.returncode}")
        sys.exit(result.returncode)


def main():
    parser = argparse.ArgumentParser(description="Cross-platform build helper for Tissu")
    parser.add_argument("--dependencies", action="store_true", help="Download build dependencies")
    parser.add_argument("--no-compile", action="store_true", help="Skip CMake configuration")
    parser.add_argument("--no-build", action="store_true", help="Skip build step")
    parser.add_argument(
        "--no-viewer",
        action="store_true",
        help="Compile the project without viewer",
    )
    parser.add_argument(
        "--no-tracy",
        action="store_true",
        help="Compile the project without Tracy profiler",
    )
    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=None,
        help="Number of parallel jobs for building (default: auto)",
    )

    args = parser.parse_args()

    # Paths
    script_dir = Path(__file__).parent.resolve()
    root_dir = script_dir.parent
    build_dir = root_dir / "build"

    # 0. Download dependencies
    if args.dependencies:
        print("--- Downloading Dependencies ---")
        run_command([sys.executable, "-m", "pip", "install", "jinja2"], cwd=root_dir)
        print()

    # 1. Compile
    if not args.no_compile:
        print("--- Configuring CMake ---")
        build_dir.mkdir(parents=True, exist_ok=True)
        cmake_cmd = [
            "cmake",
            "-S",
            str(root_dir),
            "-B",
            str(build_dir),
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
            "-DBENCHMARK_ENABLE_WERROR=OFF",
        ]
        if platform.system() != "Windows":
            cmake_cmd.append("-DCMAKE_C_FLAGS=-Wno-c2y-extensions")
        # Viewer not available
        if args.no_viewer:
            cmake_cmd.append("-DTISSU_BUILD_VIEWER=OFF")
        if args.no_tracy:
            cmake_cmd.append("-DTISSU_ENABLE_TRACY=OFF")

        run_command(cmake_cmd, cwd=root_dir)
        print()

    # 2. Build
    if not args.no_build:
        print("--- Building Project ---")
        build_cmd = [
            "cmake",
            "--build",
            str(build_dir),
            "--config",
            "Release",
        ]
        if args.jobs is not None:
            build_cmd.extend(["--parallel", str(args.jobs)])
        else:
            build_cmd.append("--parallel")

        run_command(build_cmd, cwd=root_dir)
        print()


if __name__ == "__main__":
    main()
