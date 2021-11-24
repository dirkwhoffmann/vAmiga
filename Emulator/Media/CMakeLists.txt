target_sources(vAmiga PRIVATE

AmigaFile.cpp
Snapshot.cpp
Script.cpp
HDFFile.cpp

)

add_subdirectory(RomFiles)
add_subdirectory(DiskFiles)
