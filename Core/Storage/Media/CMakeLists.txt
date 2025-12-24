target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

MediaError.cpp
MediaFile.cpp
AnyFile.cpp
DiskImage.cpp
FloppyDiskImage.cpp
HardDiskImage.cpp
FloppyFactory.cpp
Snapshot.cpp
Script.cpp
Workspace.cpp
)

add_subdirectory(Rom)
add_subdirectory(ADF)
add_subdirectory(ADZ)
add_subdirectory(EADF)
add_subdirectory(HDF)
add_subdirectory(HDZ)
add_subdirectory(IMG)
add_subdirectory(ST)
add_subdirectory(DMS)
add_subdirectory(EXE)
