target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

MediaFile.cpp
AnyFile.cpp
Snapshot.cpp
Script.cpp
Workspace.cpp

)

add_subdirectory(RomFiles)
add_subdirectory(DiskFiles)
