target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

Drive.cpp
FloppyDrive.cpp
HardDrive.cpp
FloppyDisk.cpp

)

add_subdirectory(Encoders)
