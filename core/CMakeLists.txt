set(CORE_SOURCES
    src/physics/Particle.cpp
    src/physics/Solver.cpp
    src/physics/Constraint.cpp
    src/physics/DistanceConstraint.cpp
    src/physics/BendingConstraint.cpp
    src/physics/PinConstraint.cpp
    src/physics/Collider.cpp
    src/physics/PlaneCollider.cpp
    src/physics/SphereCollider.cpp
    src/physics/Force.cpp
    src/physics/GravityForce.cpp
    src/physics/AerodynamicForce.cpp
    src/physics/SpatialHash.cpp
    src/engine/ClothMesh.cpp
    src/engine/Cloth.cpp
    src/io/OBJLoader.cpp
    src/io/ConfigLoader.cpp
    src/io/AlembicExporter.cpp
    src/utils/Logger.cpp
)

find_package(Alembic REQUIRED)
find_package(Imath REQUIRED)

add_library(ClothCore SHARED ${CORE_SOURCES})

target_link_libraries(ClothCore PUBLIC Eigen3::Eigen)
target_link_libraries(ClothCore PUBLIC tinyobjloader)
target_link_libraries(ClothCore PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(ClothCore PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(ClothCore PUBLIC 
    Alembic::Alembic 
    Imath::Imath
)


target_include_directories(ClothCore PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)