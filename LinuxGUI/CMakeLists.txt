cmake_minimum_required(VERSION 3.16)

project(vAmigaImGui)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED False)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG        docking
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    ImGuiFileDialog
    GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog
    GIT_TAG        v0.6.8
)
FetchContent_MakeAvailable(ImGuiFileDialog)
target_include_directories(ImGuiFileDialog PUBLIC ${imgui_SOURCE_DIR})
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(ImGuiFileDialog PRIVATE -Wno-deprecated-enum-enum-conversion)
endif()

add_executable(vAmigaLinux
    main.cc
    application.cc
)

target_link_libraries(vAmigaLinux PRIVATE
    VACore
    ${SDL2_LIBRARIES}
    ImGuiFileDialog
)

target_include_directories(vAmigaLinux PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${ImGuiFileDialog_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(vAmigaLinux PRIVATE
    components/console.cc
    components/dashboard.cc
    components/file_picker.cc
    components/input_manager.cc
    components/inspector.cc
    components/virtual_keyboard.cc
    services/config_provider.cc
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_compile_definitions(vAmigaLinux PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_GL3W
    USE_STD_FILESYSTEM
)

target_link_libraries(vAmigaLinux PRIVATE GL dl)
