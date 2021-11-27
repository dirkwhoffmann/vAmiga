cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

# Set the project name
project(vAmiga)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Create the emulator library
add_library(vAmiga Amiga.cpp)

# Specify compile options
if(MSVC)
  target_compile_options(vAmiga PRIVATE /W4)
  # target_compile_options(vAmiga PRIVATE /WX)
  target_compile_options(vAmiga PRIVATE /wd4100 /wd4201 /wd4458)
else()
  target_compile_options(vAmiga PRIVATE -O3 -Wall -Werror)
  target_compile_options(vAmiga PRIVATE -Wno-unused-parameter)
endif()

# Enable additional features on certain architectures
if(MSVC)
  #
else()
  target_compile_definitions(vAmiga PRIVATE SCREEN_RECORDER)
endif()

# Add include paths
include_directories(
${CMAKE_SOURCE_DIR}/.
${CMAKE_SOURCE_DIR}/..
${CMAKE_SOURCE_DIR}/Utilities
${CMAKE_SOURCE_DIR}/Base
${CMAKE_SOURCE_DIR}/LogicBoard
${CMAKE_SOURCE_DIR}/CPU
${CMAKE_SOURCE_DIR}/CPU/Moira
${CMAKE_SOURCE_DIR}/CIA
${CMAKE_SOURCE_DIR}/Paula
${CMAKE_SOURCE_DIR}/Paula/Audio
${CMAKE_SOURCE_DIR}/Paula/DiskController
${CMAKE_SOURCE_DIR}/Paula/UART
${CMAKE_SOURCE_DIR}/Denise
${CMAKE_SOURCE_DIR}/Agnus
${CMAKE_SOURCE_DIR}/Agnus/Copper
${CMAKE_SOURCE_DIR}/Agnus/Blitter
${CMAKE_SOURCE_DIR}/Memory
${CMAKE_SOURCE_DIR}/Peripherals
${CMAKE_SOURCE_DIR}/Peripherals/Drive
${CMAKE_SOURCE_DIR}/Peripherals/Mouse
${CMAKE_SOURCE_DIR}/Peripherals/Joystick
${CMAKE_SOURCE_DIR}/Peripherals/Keyboard
${CMAKE_SOURCE_DIR}/Media
${CMAKE_SOURCE_DIR}/Media/RomFiles
${CMAKE_SOURCE_DIR}/Media/DiskFiles
${CMAKE_SOURCE_DIR}/FileSystems
${CMAKE_SOURCE_DIR}/RetroShell
${CMAKE_SOURCE_DIR}/xdms)

# Add sub directories
add_subdirectory(Utilities)
add_subdirectory(Base)
add_subdirectory(LogicBoard)
add_subdirectory(CPU)
add_subdirectory(CIA)
add_subdirectory(Paula)
add_subdirectory(Denise)
add_subdirectory(Agnus)
add_subdirectory(Memory)
add_subdirectory(Peripherals)
add_subdirectory(Media)
add_subdirectory(FileSystems)
add_subdirectory(RetroShell)
add_subdirectory(xdms)

# Add libraries
target_link_libraries(vAmiga xdms)
