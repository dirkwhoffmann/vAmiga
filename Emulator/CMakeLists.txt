cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# Set the project name
project(vAmigaCore)

# Enable testing
enable_testing()

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

# Set the optimization level (if not specified on the command line)
if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE Release)
endif()

# Add the emulator library
add_library(vAmigaCore Components/Amiga.cpp config.cpp)

# Add the console app (vAmiga Headless)
add_executable(vAmigaConsole Headless.cpp config.cpp)
target_link_libraries(vAmigaConsole vAmigaCore)

# Specify compile options
target_compile_definitions(vAmigaCore PUBLIC _USE_MATH_DEFINES)
if(WIN32)
  target_link_libraries(vAmigaConsole ws2_32)
endif()
if(MSVC)
  target_compile_options(vAmigaCore PUBLIC /W4 /bigobj /Zc:preprocessor) # /WX disabled for now
  target_compile_options(vAmigaCore PUBLIC /wd4100 /wd4201 /wd4324 /wd4458 /wd4091 /wd4430)
  set_source_files_properties(CPU/Moira/Moira.cpp PROPERTIES COMPILE_FLAGS "/wd4127")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(vAmigaCore PUBLIC -Wall -Werror)
  target_compile_options(vAmigaCore PUBLIC -Wno-unused-parameter)
  target_compile_options(vAmigaCore PUBLIC -fconcepts)
  target_compile_options(vAmigaCore PUBLIC -Wno-restrict)
  # target_compile_options(vAmigaCore PUBLIC -Wno-dangling-pointer)
  target_compile_options(vAmigaCore PUBLIC -Wno-array-bounds)
else()
  target_compile_options(vAmigaCore PUBLIC -Wall -Werror)
  target_compile_options(vAmigaCore PUBLIC -Wno-unused-parameter)
  target_compile_options(vAmigaCore PUBLIC -Wno-gnu-anonymous-struct)
  target_compile_options(vAmigaCore PUBLIC -Wno-nested-anon-types)
endif()

# Add include paths
target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add sub directories
add_subdirectory(Base)
add_subdirectory(Components)
add_subdirectory(FileSystems)
add_subdirectory(Media)
add_subdirectory(Misc)
add_subdirectory(Peripherals)
add_subdirectory(ThirdParty)
add_subdirectory(Utilities)

# Add libraries
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(vAmigaCore xdms Threads::Threads)

# Add tests
add_test(NAME SelfTest COMMAND vAmigaConsole --selftest)
