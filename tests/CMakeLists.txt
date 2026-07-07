add_executable(unit_tests
    physics/test_particle.cpp
    physics/test_distance_constraint.cpp
    physics/test_pin_constraint.cpp
    physics/test_volume_constraint.cpp
    physics/test_stitch_constraint.cpp
    physics/test_constraint_graph.cpp
    data-structures/test_spatial_hash.cpp
    data-structures/test_bvh.cpp
    physics/test_plane_collider.cpp
    physics/test_sphere_collider.cpp
    physics/test_mesh_collider.cpp
    io/test_config_loader.cpp
    io/test_state_serializer.cpp
    math/test_geometry.cpp
)

add_executable(integration
	integration/test_integration.cpp	
)

target_link_libraries(unit_tests 
    PRIVATE 
        ClothCore
        GTest::gtest_main 
        Eigen3::Eigen
)

target_link_libraries(integration
    PRIVATE 
        ClothCore
        GTest::gtest_main 
        Eigen3::Eigen
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
gtest_discover_tests(integration)
