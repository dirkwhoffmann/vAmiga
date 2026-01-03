cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# Set the project name
project(VACore)

# Enable testing
list(APPEND CMAKE_CTEST_ARGUMENTS "--verbose")
enable_testing()

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

# Set the optimization level (if not specified on the command line)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Find and optionally link zlib
find_package(ZLIB QUIET)
if(ZLIB_FOUND)
  message(STATUS "Found ZLIB: ${ZLIB_LIBRARIES}")
else()
  message(STATUS "ZLIB not found. Compression support will be disabled.")
endif()

# Add the emulator library
add_library(VACore VAmiga.cpp debug.cpp)

# Add the headless app
add_executable(VAHeadless Headless.cpp debug.cpp)
target_link_libraries(VAHeadless VACore)

# Specify compile options
target_compile_definitions(VACore PUBLIC _USE_MATH_DEFINES)
if(WIN32)
  target_link_libraries(VAHeadless ws2_32)
endif()
if(MSVC)
  target_compile_options(VACore PUBLIC /W4 /WX /bigobj /Zc:preprocessor)
  target_compile_options(VACore PUBLIC /wd4100 /wd4201 /wd4458)
  # target_compile_options(VACore PUBLIC /wd4324)
  set_source_files_properties(CPU/Moira/Moira.cpp PROPERTIES COMPILE_FLAGS "/wd4127")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(VACore PUBLIC -Wall -Werror)
  target_compile_options(VACore PUBLIC -Wno-restrict)
  target_compile_options(VACore PUBLIC -Wno-unused-parameter)
# target_compile_options(VACore PUBLIC -Wno-array-bounds)
  target_compile_options(VACore PUBLIC -fconcepts)
else()
  target_compile_options(VACore PUBLIC -Wall -Werror)
  target_compile_options(VACore PUBLIC -Wno-unused-parameter)
  target_compile_options(VACore PUBLIC -Wno-gnu-anonymous-struct)
  target_compile_options(VACore PUBLIC -Wno-nested-anon-types)
endif()

# Add include paths
target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add subdirectories
add_subdirectory(utlib)
add_subdirectory(Infrastructure)
add_subdirectory(Components)
add_subdirectory(Misc)
add_subdirectory(Peripherals)
add_subdirectory(Ports)
add_subdirectory(RetroVault)
add_subdirectory(ThirdParty)

# Add libraries
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(VACore utlib xdms Threads::Threads)

# Link zlib if available
if(ZLIB_FOUND)
  target_link_libraries(VACore ZLIB::ZLIB)
  target_link_libraries(VAHeadless ZLIB::ZLIB)
  target_compile_definitions(VACore PUBLIC USE_ZLIB=1)  # Optional define for conditional compilation
endif()

#target_link_libraries(VACore lz4)
#target_link_libraries(VAHeadless lz4)

# Add tests
add_test(NAME SelfTest1 COMMAND VAHeadless --verbose --footprint)
add_test(NAME SelfTest2 COMMAND VAHeadless --verbose --smoke)
add_test(NAME SelfTest3 COMMAND VAHeadless --verbose --diagnose)
