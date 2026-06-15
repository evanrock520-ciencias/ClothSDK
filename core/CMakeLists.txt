set(CORE_SOURCES
    src/physics/Particle.cpp
    src/physics/Solver.cpp
    src/physics/Constraint.cpp
    src/physics/DistanceConstraint.cpp
    src/physics/BendingConstraint.cpp
    src/physics/PinConstraint.cpp
    src/physics/VolumeConstraint.cpp
    src/physics/StitchConstraint.cpp
    src/physics/ConstraintGraph.cpp
    src/physics/Collider.cpp
    src/physics/PlaneCollider.cpp
    src/physics/SphereCollider.cpp
    src/physics/CapsuleCollider.cpp
    src/physics/MeshCollider.cpp
    src/physics/Force.cpp
    src/physics/GravityForce.cpp
    src/physics/AerodynamicForce.cpp
    src/data-structures/SpatialHash.cpp
    src/data-structures/BVH.cpp
    src/engine/ClothMesh.cpp
    src/engine/Cloth.cpp
    src/engine/World.cpp
    src/io/OBJLoader.cpp
    src/io/OBJExporter.cpp
    src/io/ConfigLoader.cpp
    src/io/SceneLoader.cpp
    src/io/SceneExporter.cpp
    src/io/AlembicExporter.cpp
    src/io/StateSerializer.cpp
    src/utils/Logger.cpp
)

find_package(Alembic REQUIRED)
find_package(Imath REQUIRED)

add_library(ClothCore STATIC ${CORE_SOURCES})

if(MSVC)
    target_compile_options(ClothCore PRIVATE /openmp)
endif()

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