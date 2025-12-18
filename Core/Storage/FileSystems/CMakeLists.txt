target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

BootBlockImage.cpp
FileSystem.cpp
FileSystemLayer1.cpp
FileSystemLayer2.cpp
FileSystemLayer3.cpp
FSAllocator.cpp
FSBlock.cpp
FSCache.cpp
FSError.cpp
FSExtension.cpp
FSDescriptor.cpp
FSDoctor.cpp
FSExporter.cpp
FSImporter.cpp
FSObjects.cpp
FSTree.cpp

)
