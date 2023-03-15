target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

AmigaFile.cpp
Snapshot.cpp
Script.cpp

)

add_subdirectory(RomFiles)
add_subdirectory(DiskFiles)
