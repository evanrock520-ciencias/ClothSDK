import platform
import subprocess
from pathlib import Path

import duckdb


def find_benchmark_executable(repo_root: Path) -> Path:
    exe_name = "tissu_benchmarks.exe" if platform.system() == "Windows" else "tissu_benchmarks"
    build_dir = repo_root / "build"

    candidate_paths = [
        build_dir / "bin" / "Release" / exe_name,
        build_dir / "bin" / exe_name,
        build_dir / "bin" / "Debug" / exe_name,
        build_dir / "bin" / "RelWithDebInfo" / exe_name,
        build_dir / "benchmarks" / exe_name,
        build_dir / "benchmarks" / "Release" / exe_name,
    ]

    for candidate in candidate_paths:
        if candidate.exists():
            return candidate.resolve()

    raise FileNotFoundError("Benchmark executable not found. Please ensure the project is built correctly.")


def get_git_info() -> dict:
    commit_hash = subprocess.check_output(["git", "rev-parse", "HEAD"]).decode().strip()
    branch_name = subprocess.check_output(["git", "rev-parse", "--abbrev-ref", "HEAD"]).decode().strip()
    timestamp = subprocess.check_output(["git", "show", "-s", "--format=%ct", commit_hash]).decode().strip()
    return {"commit_hash": commit_hash, "branch_name": branch_name, "timestamp": timestamp}


def run_benchmarks(bench_exe: Path, output_json: Path):
    command = [str(bench_exe), "--benchmark_format=json", f"--benchmark_out={output_json}"]
    subprocess.run(command, check=True)


def json_to_parquet(json_file: Path, parquet_file: Path, git_info: dict):
    json_str = str(json_file).replace("\\", "/")
    parquet_str = str(parquet_file).replace("\\", "/")

    duckdb.execute(f"""
        COPY (
            SELECT
                '{git_info['commit_hash']}' AS commit_hash,

                '{git_info['branch_name']}' AS branch_name,
                to_timestamp({git_info['timestamp']}) AS commit_timestamp,
                b.name AS benchmark_name,
                b.real_time,
                b.cpu_time,
                b.time_unit,
                b.iterations
            FROM (
                SELECT unnest(benchmarks) AS b
                FROM read_json_auto('{json_str}')
            )
        ) TO '{parquet_str}' (FORMAT PARQUET);
    """)


def main():
    repo_root = Path(__file__).parent.parent.parent.resolve()
    bench_exe = find_benchmark_executable(repo_root)
    git_info = get_git_info()

    parquet_dir = repo_root / "benchmarks" / "storage" / "parquet"
    parquet_dir.mkdir(parents=True, exist_ok=True)

    output_json = parquet_dir.parent / "temp_results.json"
    output_parquet = parquet_dir / f"{git_info['branch_name']}_{git_info['commit_hash']}.parquet"

    try:
        run_benchmarks(bench_exe, output_json)
        json_to_parquet(output_json, output_parquet, git_info)
    finally:
        if output_json.exists():
            output_json.unlink()


if __name__ == "__main__":
    main()
