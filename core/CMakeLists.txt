include_directories(include)

set(CORE_SOURCES
    src/physics/Particle.cpp
    src/physics/Solver.cpp
)

add_library(ClothCore SHARED ${CORE_SOURCES})

target_link_libraries(ClothCore PUBLIC Eigen3::Eigen)

target_include_directories(ClothCore PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)