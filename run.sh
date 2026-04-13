#!/bin/bash
set -e

COMPILE=true
BUILD=true
TEST=true
RUN=true
BENCH=false

for arg in "$@"; do
    case $arg in
        --no-build) BUILD=false ;;
        --no-test)  TEST=false ;;
        --no-run)   RUN=false ;;
        --no-compile) COMPILE=false;;
	    --bench) BENCH=true;;
    esac
done

if $COMPILE; then
    echo "Compiling..."
    echo ""
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cd ..
    echo ""
fi

if $BUILD; then
    echo "Building..."
    echo ""
    cd build && make -j$(nproc) && cd ..
fi

if $TEST; then
    echo "Testing..."
    echo ""
    ./build/tests/unit_tests
    echo ""
fi

if $BENCH; then
    echo "Running Benchmarks..."
    echo ""
    ./benchmarks/run_benchmarks.sh
    echo ""
fi

if $RUN; then
    echo "Running..."
    echo ""
    python3 -m examples.simulation
    echo ""
fi
