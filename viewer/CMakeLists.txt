find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_BACKENDS_DIR ${IMGUI_DIR}/backends)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp
)

add_library(imgui_lib STATIC ${IMGUI_SOURCES})
target_include_directories(imgui_lib PUBLIC ${IMGUI_DIR} ${IMGUI_BACKENDS_DIR})
target_link_libraries(imgui_lib PUBLIC glfw OpenGL::GL)
set_target_properties(imgui_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(ViewerCore STATIC
    src/Application.cpp
    src/Renderer.cpp
    src/Camera.cpp
)

target_link_libraries(ViewerCore 
    PUBLIC 
        ClothCore 
        imgui_lib 
        glad 
        glfw 
        OpenGL::GL 
        Threads::Threads
)

target_include_directories(ViewerCore PUBLIC include)

set_target_properties(ViewerCore PROPERTIES POSITION_INDEPENDENT_CODE ON)
