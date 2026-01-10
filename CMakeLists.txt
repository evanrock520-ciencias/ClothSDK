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

FetchContent_Declare(
  tinyobjloader
  GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
  GIT_TAG        release
)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)

FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.13.6
)

set(TINYOBJLOADER_INSTALL OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(PYBIND11_INSTALL OFF CACHE BOOL "" FORCE)
set(PYBIND11_TEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest eigen tinyobjloader json pybind11)

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

add_executable(main main.cpp)
target_link_libraries(main PRIVATE ClothCore GTest::gtest_main)

pybind11_add_module(cloth_sdk python/src/bindings.cpp)
target_link_libraries(cloth_sdk PRIVATE ClothCore)

if(BUILD_MAYA_PLUGIN)
    add_subdirectory(maya)
endif()

enable_testing()
add_subdirectory(tests)