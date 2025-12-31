file(GLOB_RECURSE TEST_SOURCES "core_tests/*.cpp")

add_executable(unit_tests ${TEST_SOURCES})

target_link_libraries(unit_tests 
    PRIVATE 
        ClothCore 
        GTest::gtest_main 
        Eigen3::Eigen
)

include(GoogleTest)
gtest_discover_tests(unit_tests)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/particle_test.cpp")
    add_executable(particle_test core/particle_test.cpp)
    target_link_libraries(particle_test PRIVATE ClothCore GTest::gtest_main)

endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/distance_constraint_test.cpp")
    add_executable(distance_test core/distance_constraint_test.cpp)
    target_link_libraries(distance_test PRIVATE ClothCore GTest::gtest_main)

endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/bending_constraint_test.cpp")
    add_executable(bending_test core/bending_constraint_test.cpp)
    target_link_libraries(bending_test PRIVATE ClothCore GTest::gtest_main)

endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/solver_test.cpp")
    add_executable(solver_test core/solver_test.cpp)
    target_link_libraries(solver_test PRIVATE ClothCore GTest::gtest_main)

endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/spatial_hash_test.cpp")
    add_executable(spatial_hash_test core/spatial_hash_test.cpp)
    target_link_libraries(solver_test PRIVATE ClothCore GTest::gtest_main)

endif()