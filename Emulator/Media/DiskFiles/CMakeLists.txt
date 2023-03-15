target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

DiskFile.cpp
FloppyFile.cpp
HDFFile.cpp
ADFFile.cpp
EXTFile.cpp
IMGFile.cpp
DMSFile.cpp
EXEFile.cpp
Folder.cpp

)
