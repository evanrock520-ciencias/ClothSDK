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

target_include_directories(imgui_lib PUBLIC 
    ${IMGUI_DIR} 
    ${IMGUI_BACKENDS_DIR}
)
target_link_libraries(imgui_lib PUBLIC glfw OpenGL::GL)

add_executable(ClothViewer 
    main.cpp
    src/Application.cpp
    src/Renderer.cpp
    src/Camera.cpp
)

target_link_libraries(ClothViewer 
    PRIVATE 
        ClothCore        
        imgui_lib        
        glfw             
        glad             
        OpenGL::GL      
        Threads::Threads 
)

target_include_directories(ClothViewer PRIVATE include)
