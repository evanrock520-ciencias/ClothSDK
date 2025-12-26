cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

project(ClothSDK VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_MAYA_PLUGIN "Build the Maya plugin" OFF)

find_package(Eigen3 3.3 REQUIRED)

if(BUILD_MAYA_PLUGIN)
    if(NOT DEFINED ENV{MAYA_DEVKIT_BASE})
        message(WARNING "MAYA_DEVKIT_BASE not set. Maya plugin will not be built.")
        set(BUILD_MAYA_PLUGIN OFF)
    else()
        set(MAYA_SDK_INCLUDE_DIR "$ENV{MAYA_DEVKIT_BASE}/include")
        set(MAYA_SDK_LIBRARY_DIR "$ENV{MAYA_DEVKIT_BASE}/lib")
    endif()
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2  
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_subdirectory(core)

if(BUILD_MAYA_PLUGIN)
    add_subdirectory(maya)
endif()