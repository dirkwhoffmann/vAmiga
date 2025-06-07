target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

FSDescriptors.cpp
FileSystem.cpp
MutableFileSystem.cpp
FSObjects.cpp
FSBlock.cpp
FSPath.cpp
BootBlockImage.cpp

)
