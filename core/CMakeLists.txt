set(CORE_SOURCES
    src/physics/Particle.cpp
    src/physics/Solver.cpp
    src/physics/Constraint.cpp
    src/physics/DistanceConstraint.cpp
    src/physics/BendingConstraint.cpp
    src/physics/Collider.cpp
    src/physics/PlaneCollider.cpp
    src/engine/ClothMesh.cpp
)

add_library(ClothCore SHARED ${CORE_SOURCES})

target_link_libraries(ClothCore PUBLIC Eigen3::Eigen)

target_include_directories(ClothCore PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)