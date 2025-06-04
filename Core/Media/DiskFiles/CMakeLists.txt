target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

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
