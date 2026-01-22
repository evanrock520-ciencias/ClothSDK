cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

project(ClothSDK VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)

FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.36
)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.91.5-docking 
)

set(TINYOBJLOADER_INSTALL OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(PYBIND11_INSTALL OFF CACHE BOOL "" FORCE)
set(PYBIND11_TEST OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest eigen tinyobjloader json pybind11 glfw glad imgui)
find_package(OpenMP REQUIRED)

add_subdirectory(core)
add_subdirectory(viewer)

pybind11_add_module(cloth_sdk python/src/bindings.cpp)
target_link_libraries(cloth_sdk PRIVATE ClothCore ViewerCore)

set(STUB_OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/cloth_sdk.pyi
)

find_program(STUBGEN_EXECUTABLE stubgen REQUIRED)

add_custom_command(
    OUTPUT ${STUB_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E rm -f ${STUB_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E env
            "PYTHONPATH=$<TARGET_FILE_DIR:cloth_sdk>"
            ${STUBGEN_EXECUTABLE}
            -m cloth_sdk
            --output ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS cloth_sdk
    WORKING_DIRECTORY $<TARGET_FILE_DIR:cloth_sdk>
    COMMENT "Generating Python stubs for cloth_sdk"
)

add_custom_target(cloth_sdk_stubs ALL
    DEPENDS ${STUB_OUTPUT}
)

enable_testing()