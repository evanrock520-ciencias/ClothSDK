cmake_minimum_required(VERSION 3.10)
project(ClothSDK VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Used packages
find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)

# Directories
add_subdirectory(core)