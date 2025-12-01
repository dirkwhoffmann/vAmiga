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
FloppyFactory.cpp
ADFFactory.cpp
ADZFactory.cpp
HDFFactory.cpp
HDZFactory.cpp
IMGFactory.cpp
DMSFactory.cpp
EXEFactory.cpp
STFactory.cpp
ADFEncoder.cpp
ADZEncoder.cpp
HDFEncoder.cpp
HDZEncoder.cpp
IMGEncoder.cpp
DMSEncoder.cpp
EXEEncoder.cpp
STEncoder.cpp
)
