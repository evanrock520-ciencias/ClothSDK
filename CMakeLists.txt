cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

project(ClothSDK VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2  
)

FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG        3.4.0 
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest eigen)

option(BUILD_MAYA_PLUGIN "Build the Maya plugin" OFF)

if(BUILD_MAYA_PLUGIN)
    if(NOT DEFINED ENV{MAYA_DEVKIT_BASE})
        message(WARNING "MAYA_DEVKIT_BASE not set. Maya plugin will not be built.")
        set(BUILD_MAYA_PLUGIN OFF)
    else()
        set(MAYA_SDK_INCLUDE_DIR "$ENV{MAYA_DEVKIT_BASE}/include")
        set(MAYA_SDK_LIBRARY_DIR "$ENV{MAYA_DEVKIT_BASE}/lib")
    endif()
endif()

add_subdirectory(core)

add_executable(unit_tests tests/physics_tests.cpp)
target_link_libraries(unit_tests PRIVATE ClothCore GTest::gtest_main)
add_test(NAME ALLPhysicsTests COMMAND unit_tests)

add_executable(test_xpbd tests/test_xpbd.cpp)
target_link_libraries(test_xpbd PRIVATE ClothCore Eigen3::Eigen)

if(BUILD_MAYA_PLUGIN)
    add_subdirectory(maya)
endif()

enable_testing()