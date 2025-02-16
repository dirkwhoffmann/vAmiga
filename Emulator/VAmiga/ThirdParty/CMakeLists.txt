target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(xdms)

add_library(lz4 lz4.c lz4.h)
