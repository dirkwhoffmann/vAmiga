target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

BlockStorage.cpp
BootBlockImage.cpp
FileSystem.cpp
FSDescriptors.cpp
FSObjects.cpp
FSBlock.cpp
FSNode.cpp
FSTree.cpp
MutableFileSystem.cpp

)
