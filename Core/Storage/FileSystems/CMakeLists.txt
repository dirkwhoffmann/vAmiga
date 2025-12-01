target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

BootBlockImage.cpp
FileSystem.cpp
FileSystemLayer0.cpp
FileSystemLayer1.cpp
FileSystemLayer2.cpp
FileSystemFactory.cpp
FSAllocator.cpp
FSBlock.cpp
FSExtension.cpp
FSDescriptor.cpp
FSDoctor.cpp
FSExporter.cpp
FSImporter.cpp
FSObjects.cpp
FSStorage.cpp
FSTree.cpp

)
