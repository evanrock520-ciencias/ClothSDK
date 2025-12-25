file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(cloth_sim ${SOURCES})

target_include_directories(cloth_sim PRIVATE include)

target_link_libraries(cloth_sim PRIVATE glm::glm nlohmann_json::nlohmann_json)

set_target_properties(cloth_sim PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")