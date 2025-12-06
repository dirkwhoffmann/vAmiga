target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

DeviceDescriptors.cpp
DeviceError.cpp

)

add_subdirectory(FileSystems)
add_subdirectory(Media)
