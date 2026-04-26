cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

project(
	Tissu 
	VERSION 1.0.0 
	LANGUAGES CXX
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Tissu Dependencies
FetchContent_Declare(
	googletest 
	GIT_REPOSITORY https://github.com/google/googletest.git 
	GIT_TAG v1.15.2
)

FetchContent_Declare(
	googlebenchmark 
	GIT_REPOSITORY https://github.com/google/benchmark.git 
	GIT_TAG v1.9.1
)

FetchContent_Declare(
	eigen 
	GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git 
	GIT_TAG 3.4.0
)

FetchContent_Declare(
	tinyobjloader 
	GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git 
	GIT_TAG release
)

FetchContent_Declare(
	json 
	GIT_REPOSITORY https://github.com/nlohmann/json.git 
	GIT_TAG v3.11.3
)

FetchContent_Declare(
	pybind11 
	GIT_REPOSITORY https://github.com/pybind/pybind11.git 
	GIT_TAG v2.13.6
)

set(TINYOBJLOADER_INSTALL OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(PYBIND11_INSTALL OFF CACHE BOOL "" FORCE)
set(PYBIND11_TEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest eigen tinyobjloader json pybind11 googlebenchmark)

find_package(OpenMP REQUIRED)

# Viewer
option(TISSU_BUILD_VIEWER "Build the OpenGL viewer" ON)
if(TISSU_BUILD_VIEWER)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

	# Viewer Dependencies
    FetchContent_Declare(
		glfw 
		GIT_REPOSITORY https://github.com/glfw/glfw.git 
		GIT_TAG 3.3.8
	)

    FetchContent_Declare(
		glad 
		GIT_REPOSITORY https://github.com/Dav1dde/glad.git 
		GIT_TAG 
		v0.1.36
	)

    FetchContent_Declare(
		imgui 
		GIT_REPOSITORY https://github.com/ocornut/imgui.git 
		GIT_TAG v1.91.5-docking
	)

    FetchContent_MakeAvailable(glfw glad imgui)
endif()

add_subdirectory(core)

if(TISSU_BUILD_VIEWER)
    add_subdirectory(viewer)
endif()

# Python bindings library 
if(TISSU_BUILD_VIEWER)
    pybind11_add_module(_cloth_sdk_core python/src/bindings.cpp)
    target_link_libraries(_cloth_sdk_core PRIVATE ClothCore ViewerCore)
else()
    pybind11_add_module(_cloth_sdk_core python/src/bindings_headless.cpp)
    target_link_libraries(_cloth_sdk_core PRIVATE ClothCore)
endif()

set(STUB_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/_cloth_sdk_core.pyi)

# Python stubs
find_program(STUBGEN_EXECUTABLE stubgen REQUIRED)

add_custom_command(
    OUTPUT ${STUB_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E rm -f ${STUB_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E env
            "PYTHONPATH=$<TARGET_FILE_DIR:_cloth_sdk_core>"
            ${STUBGEN_EXECUTABLE}
            -m _cloth_sdk_core
            --output ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS _cloth_sdk_core
    WORKING_DIRECTORY $<TARGET_FILE_DIR:_cloth_sdk_core>
    COMMENT "Generating Python stubs for _cloth_sdk_core"
)

add_custom_target(_cloth_sdk_core_stubs ALL DEPENDS ${STUB_OUTPUT})

# Moving library
add_custom_command(
    TARGET _cloth_sdk_core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:_cloth_sdk_core>
        ${CMAKE_SOURCE_DIR}/python/tissu/$<TARGET_FILE_NAME:_cloth_sdk_core>
    COMMENT "Copying _cloth_sdk_core to python/tissu/"
)

# Moving stubs 
add_custom_command(
    TARGET _cloth_sdk_core_stubs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${STUB_OUTPUT}
        ${CMAKE_SOURCE_DIR}/python/tissu/_cloth_sdk_core.pyi
    COMMENT "Copying stubs to python/tissu/"
)

enable_testing()
add_subdirectory(tests)

add_subdirectory(benchmarks)
