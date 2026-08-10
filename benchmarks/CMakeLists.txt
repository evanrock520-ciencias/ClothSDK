add_executable(tissu_benchmarks
    bench_solver.cpp
)

target_link_libraries(tissu_benchmarks
    PRIVATE TissuCore benchmark::benchmark
)