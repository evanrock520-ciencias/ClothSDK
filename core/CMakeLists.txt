set(CORE_SOURCES
    src/physics/Particle.cpp
    src/physics/Solver.cpp
    src/physics/Constraint.cpp
    src/physics/DistanceConstraint.cpp
    src/physics/BendingConstraint.cpp
    src/physics/Collider.cpp
    src/physics/PlaneCollider.cpp
    src/physics/SphereCollider.cpp
    src/physics/SpatialHash.cpp
    src/engine/ClothMesh.cpp
    src/io/OBJLoader.cpp
    src/io/ConfigLoader.cpp
    src/utils/Logger.cpp
)

add_library(ClothCore SHARED ${CORE_SOURCES})

target_link_libraries(ClothCore PUBLIC Eigen3::Eigen)
target_link_libraries(ClothCore PUBLIC tinyobjloader)
target_link_libraries(ClothCore PUBLIC nlohmann_json::nlohmann_json)


target_include_directories(ClothCore PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)