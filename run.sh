#!/bin/bash
set -e

COMPILE=true
BUILD=true
TEST=true
INTEGRATION=true
RUN=true
BENCH=false

for arg in "$@"; do
	case $arg in
	--no-build) BUILD=false ;;
	--no-test) TEST=false ;;
	--no-run) RUN=false ;;
	--no-compile) COMPILE=false ;;
	--no-integration) INTEGRATION=false ;;
	--bench) BENCH=true ;;
	esac
done

if $COMPILE; then
	echo "Compiling..."
	echo ""
	mkdir -p build
	cd build
	cmake .. -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
		-DBENCHMARK_ENABLE_WERROR=OFF \
		-DCMAKE_C_FLAGS="-Wno-c2y-extensions"
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

if $INTEGRATION; then
	echo "Testing Integration..."
	echo ""
	./build/tests/integration
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
