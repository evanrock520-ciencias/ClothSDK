add_executable(tissu_benchmarks
    bench_aerodynamic.cpp
)

target_link_libraries(tissu_benchmarks
    PRIVATE ClothCore benchmark::benchmark
)