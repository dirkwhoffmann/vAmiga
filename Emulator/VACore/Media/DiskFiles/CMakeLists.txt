target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

DiskFile.cpp
FloppyFile.cpp
ADFFile.cpp
ADZFile.cpp
EADFFile.cpp
HDFFile.cpp
HDZFile.cpp
IMGFile.cpp
STFile.cpp
DMSFile.cpp
EXEFile.cpp

)
