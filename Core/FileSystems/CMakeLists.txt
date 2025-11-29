target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

BootBlockImage.cpp
FileSystem.cpp
FileSystemMutation.cpp
FSAllocator.cpp
FSBlock.cpp
FSComponent.cpp
FSDescriptor.cpp
FSDoctor.cpp
FSExporter.cpp
FSImporter.cpp
FSObjects.cpp
FSStorage.cpp
FSTree.cpp

)
