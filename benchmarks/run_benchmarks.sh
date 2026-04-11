#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BINARY="$SCRIPT_DIR/../build/benchmarks/tissu_benchmarks"
RESULTS_DIR="$SCRIPT_DIR/results"
TIMESTAMP=$(date +"%Y-%m-%d_%H-%M")
MACHINE=$(hostname)
OUTPUT="$RESULTS_DIR/${TIMESTAMP}_${MACHINE}.json"

mkdir -p $RESULTS_DIR

$BINARY \
    --benchmark_format=json \
    --benchmark_out=$OUTPUT \
    --benchmark_out_format=json \
    --benchmark_repetitions=3 \
    --benchmark_report_aggregates_only=true \
    > /dev/null

echo "Results saved to $OUTPUT"

