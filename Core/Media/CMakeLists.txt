target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

MediaError.cpp
AnyFile.cpp
Snapshot.cpp
Script.cpp
Workspace.cpp
)

add_subdirectory(Rom)
