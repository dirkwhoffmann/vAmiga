target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

FileSystem.cpp
MutableFileSystem.cpp
BootBlockImage.cpp
FSBlock.cpp
FSDescriptor.cpp
FSDoctor.cpp
FSObjects.cpp
FSStorage.cpp
FSTree.cpp

)
