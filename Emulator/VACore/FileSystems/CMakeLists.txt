target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

FSDescriptors.cpp
FileSystem.cpp
MutableFileSystem.cpp
FSObjects.cpp
FSBlock.cpp
BootBlockImage.cpp

)
